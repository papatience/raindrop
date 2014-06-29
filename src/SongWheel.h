
#ifndef SONGWHEEL_H_
#define SONGWHEEL_H_

#include <boost/function.hpp>

namespace dotcur
{
	class Song;
}

namespace VSRG
{
	class Song;
}

class BitmapFont;
class GraphObject2D;

namespace Game 
{

	typedef boost::function<void (Game::Song*, uint8)> SongNotification;

	typedef boost::function <float (float)> ListTransformFunction;

class SongWheel
{
private:
	SongWheel();

	int CursorPos, OldCursorPos;
	ModeType CurrentMode;

	BitmapFont* mFont;

	std::vector<dotcur::Song*> SongList;
	std::vector<VSRG::Song*> SongList7K;

	float CurrentVerticalDisplacement;
	float PendingVerticalDisplacement;

	GraphObject2D* SelCursor;

	SongNotification OnSongChange;
	SongNotification OnSongSelect;
	ListTransformFunction Transform;

	enum
	{
		M_LESSTHAN, 
		M_GREATERTHAN,
		M_RANGE
	} IntervalType;

	float RangeStart, RangeEnd;

	bool dotcurModeActive;
	bool VSRGModeActive;

	float ItemHeight;
	float Time;
	float DisplacementSpeed;

	void DisplayItem(String Text, Vec2 Position);

	bool IsInitialized;

	uint8 DifficultyIndex;

public:

	// Singleton
	static SongWheel& GetInstance();

	void Initialize(float Start, float End, bool IsDotcurActive, bool IsVSRGActive, ListTransformFunction FuncTransform, SongNotification FuncNotify, SongNotification FuncNotifySelect);
	bool HandleInput(int32 key, KeyEventType code, bool isMouseInput);
	bool HandleScrollInput(const double dx, const double dy);
	Game::Song* GetSelectedSong();
	void ChangeMode (const ModeType NewMode);
	void ReloadSongs();

	void SetFont(Directory FontDirectory);
	void SetItemHeight(float Height);

	void Update(float Delta);
	void Render();
};

}

#endif