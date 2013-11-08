/* Song Timing functions. */
#include "Global.h"
#include "Song.h"

using namespace SongInternal;
using std::vector;

float spb(float bpm)
{
	return 60 / bpm;
}

float bps(float bpm)
{
	return bpm / 60;
}

/* Assume these are sorted. */
// Return the (vector's index+1) Beat resides in.
uint32 SectionIndex(Difficulty &Diff, float Beat)
{
	vector<Difficulty::TimingSegment> &Timing = Diff.Timing;
	uint32 Index = 0;
	for (vector<Difficulty::TimingSegment>::iterator i = Timing.begin(); i != Timing.end(); i++)
	{
		if (Beat >= i->Time)
			Index++;
		else
			break;
	}
	return Index;
}

double BpmAtBeat(Difficulty &Diff, float Beat)
{
	return Diff.Timing[SectionIndex(Diff,Beat)-1].Value;
}

double TimeAtBeat(Difficulty &Diff, float Beat)
{
	vector<Difficulty::TimingSegment> &Timing = Diff.Timing;
	uint32 CurrentIndex = SectionIndex(Diff, Beat);
	double Time = Diff.Offset;

	if (Beat == 0) return Time;

	for (uint32 i = 0; i < CurrentIndex; i++)
	{	
		if (i+1 < CurrentIndex) // Get how long the current timing goes.
		{
			float BeatDurationOfSectionI = Timing[i+1].Time - Timing[i].Time; // Section lasts this much.
			float SectionDuration = BeatDurationOfSectionI * spb ( Timing[i].Value );

			if (Beat < Timing[i+1].Time && Beat > Timing[i].Time)
			{
				// If this is our interval, stop summing time of previous intervals before this one
				SectionDuration = (Beat - Timing[i].Time) * spb ( Timing[i].Value );
			}
			Time += SectionDuration;
		}else
			Time += (Beat - Timing[i].Time) * spb ( Timing[i].Value );
	}
	return Time;
}

double DifficultyDuration(Song &MySong, Difficulty &Diff)
{
	if (Diff.Measures.size())
		return TimeAtBeat(Diff, Diff.Measures.size() * MySong.MeasureLength);
	return 0;
}

void GetTimingChangesInInterval(vector<Difficulty::TimingSegment> Timing, double PointA, double PointB, vector<Difficulty::TimingSegment> &Out)
{
	Out.clear();

	for (vector<Difficulty::TimingSegment>::iterator i = Timing.begin(); i != Timing.end(); i++)
	{
		if (i->Time >= PointA && i->Time < PointB)
		{
			Out.push_back(*i);
		}
	}
}

void CalculateBarlineRatios(Song &MySong, Difficulty &Diff)
{
	vector<Difficulty::TimingSegment> &Timing = Diff.Timing;
	vector<Difficulty::TimingSegment> &Ratios = Diff.BarlineRatios;
	vector<Difficulty::TimingSegment> ChangesInInterval;

	Ratios.clear();

	for (uint32 Measure = 0; Measure < Diff.Measures.size(); Measure++)
	{
		double CurrentBeat = Measure * MySong.MeasureLength;
		double NextBeat = (Measure+1) * MySong.MeasureLength;
		double MeasureDuration = TimeAtBeat(Diff, (Measure+1)*MySong.MeasureLength) - TimeAtBeat(Diff, Measure*MySong.MeasureLength);

		GetTimingChangesInInterval(Timing, CurrentBeat, NextBeat, ChangesInInterval);

		if (ChangesInInterval.size() == 0)
		{
			double Ratio = 1/MeasureDuration;
			Difficulty::TimingSegment New;

			New.Value = Ratio;
			New.Time = TimeAtBeat(Diff, CurrentBeat);

			if (!Ratios.size())
				Ratios.push_back(New);
			else
			{
				if (Ratios.back().Value != Ratio) /* Don't add redundant ratios.*/
					Ratios.push_back(New);
			}
		}else
		{
			// Real show time on calculations is here.
			for (vector<Difficulty::TimingSegment>::iterator i = ChangesInInterval.begin(); i != ChangesInInterval.end(); i++)
			{
				Difficulty::TimingSegment New;
				double Duration;
				double DurationSeconds;
				double Fraction;
				double Ratio;

				/* Calculate how long this change lasts. */
				
				if ((i+1) != ChangesInInterval.end())
				{
					Duration = (i+1)->Time - i->Time;
				}else
				{
					Duration = NextBeat - i->Time;
				}

				/* Assign the time the change starts
				which we can assume i->Time >= CurrentBeat before this. */
				CurrentBeat = i->Time;

				/* Calculate how much the barline would move in 1 second */
				Fraction = Duration / MySong.MeasureLength;
				
				/* t/b * b = t */ 
				DurationSeconds = spb(i->Value) * Duration;

				if (DurationSeconds == 0)
					continue;

				/* f/d = r/1 (where 1 and d are 1 second and d seconds) */
				Ratio = Fraction / DurationSeconds;

				/* create new segment at i->Time */
				New.Value = Ratio;
				New.Time = TimeAtBeat(Diff, CurrentBeat);

				if (!Ratios.size())
					Ratios.push_back(New);
				else
				{
					if (Ratios.back().Value != Ratio) /* Don't add redundant ratios.*/
						Ratios.push_back(New);
				}

			}

		}
	}
}