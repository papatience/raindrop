#include <stdio.h>
#include "GameGlobal.h"
#include "GameState.h"
#include "Logging.h"
#include "SongDC.h"
#include "Song7K.h"
#include "GameWindow.h"
#include "SongLoader.h"
#include "SongWheel.h"
#include "GraphObject2D.h"
#include "ImageLoader.h"
#include "BitmapFont.h"
#include "TruetypeFont.h"
#include "SongList.h"

#include "SongDatabase.h"

using namespace Game;

SongWheel::SongWheel()
{
	Transform = NULL;
	OnSongChange = NULL;
	IsInitialized = false;
	mFont = NULL;
	PendingVerticalDisplacement = 0;
	mLoadMutex = NULL;
	mLoadThread = NULL;
	CurrentVerticalDisplacement = 0;
}

SongWheel& SongWheel::GetInstance()
{
	static SongWheel *WheelInstance = new SongWheel();
	return *WheelInstance;
}

void SongWheel::Initialize(float Start, float End, bool IsDotcurActive, bool IsVSRGActive,
	ListTransformFunction FuncTransform, SongNotification FuncNotify, SongNotification FuncNotifySelect,
	SongDatabase* Database)
{
	dotcurModeActive = IsDotcurActive;
	VSRGModeActive = IsVSRGActive;

	OnSongChange = FuncNotify;
	OnSongSelect = FuncNotifySelect;
	Transform = FuncTransform;

	DB = Database;
	if (IsInitialized)
		return;

	ListRoot = NULL;
	CurrentList = NULL;

	RangeStart = Start;
	RangeEnd = End;

	CursorPos = 0;
	OldCursorPos = 0;
	Time = 0;
	DisplacementSpeed = 2;

	Item = new GraphObject2D;
	Item->SetImage(GameState::GetInstance().GetSkinImage("item.png"));
	ItemHeight = Item->GetHeight();
	Item->SetZ(16);

	SelCursor = new GraphObject2D;
	SelCursor->SetImage(GameState::GetInstance().GetSkinImage("songselect_cursor.png"));
	SelCursor->SetSize(ItemHeight);

	ItemTextOffset = Vec2(Configuration::GetSkinConfigf("X", "ItemTextOffset"), Configuration::GetSkinConfigf("Y", "ItemTextOffset"));

	IsInitialized = true;
	DifficultyIndex = 0;

	//mFont = new BitmapFont();
	//mFont->LoadSkinFontImage("font-wheel.tga", Vec2(10, 20), Vec2(32, 32), Vec2(10,20), 32);

	mTFont = new TruetypeFont(GameState::GetInstance().GetSkinPrefix() / "font.ttf", 20);
	ReloadSongs();
}

class LoadThread
{
	boost::mutex* mLoadMutex;
	SongDatabase* DB;
	SongList* ListRoot;
	bool VSRGActive;
	bool DCActive;
public:
	LoadThread(boost::mutex* m, SongDatabase* d, SongList* r, bool va, bool da)
		: mLoadMutex(m),
		DB(d),
		ListRoot(r),
		VSRGActive(va),
		DCActive(da)
	{
	}

	void Load()
	{
		std::vector<String> Directories;
		Configuration::GetConfigListS("SongDirectories", Directories);

		SongLoader Loader (DB);

		DB->StartTransaction();

		for (std::vector<String>::iterator i = Directories.begin();
			i != Directories.end();
			i++)
		{
			ListRoot->AddDirectory (*mLoadMutex, &Loader, *i, VSRGActive, DCActive);
		}

		DB->EndTransaction();
		Log::Printf("Finished reloading songs.\n");
	}
};

void SongWheel::ReloadSongs()
{
	if (mLoadThread)
	{
		mLoadThread->join();
		delete mLoadThread;
	}

	delete ListRoot;

	ListRoot = new SongList();
	CurrentList = ListRoot;

	if (!mLoadMutex)
		mLoadMutex = new boost::mutex;

	LoadThread L(mLoadMutex, DB, ListRoot, VSRGModeActive, dotcurModeActive);
	mLoadThread = new boost::thread(&LoadThread::Load, L);
}

int SongWheel::GetCursorIndex()
{
	size_t Size;
	{
		boost::mutex::scoped_lock lock(*mLoadMutex);
		Size = CurrentList->GetNumEntries();
	}

	if (Size)
	{
		int ret = CursorPos % (int)Size;
		while (ret < 0)
			ret += Size;
		return ret;
	}
	else
		return 0;
}

bool SongWheel::HandleInput(int32 key, KeyEventType code, bool isMouseInput)
{
	uint8 max_index = 0;

	if (code == KE_Press)
	{
		switch (BindingsManager::TranslateKey(key))
		{

		case KT_Up:
			CursorPos--;
			return true;
		case KT_Down:
			CursorPos++;
			return true;
		case KT_Left:
			if (!CurrentList->IsDirectory(GetCursorIndex()))
			{
				DifficultyIndex--;
				max_index = 0;
				if (CurrentList->GetSongEntry(GetCursorIndex())->Mode == MODE_7K)
				{
					max_index = ((VSRG::Song*)CurrentList->GetSongEntry(GetCursorIndex()))->Difficulties.size() - 1;
				}else
				{
					max_index = ((dotcur::Song*)CurrentList->GetSongEntry(GetCursorIndex()))->Difficulties.size() - 1;
				}
			}

			DifficultyIndex = min(max_index, DifficultyIndex);
			return true;
		case KT_Right:
			if (!CurrentList->IsDirectory(GetCursorIndex()))
			{
				DifficultyIndex++;
				if (CurrentList->GetSongEntry(GetCursorIndex())->Mode == MODE_7K)
				{
					if (DifficultyIndex >= ((VSRG::Song*)CurrentList->GetSongEntry(GetCursorIndex()))->Difficulties.size())
						DifficultyIndex = 0;
				}else
				{
					if (DifficultyIndex >= ((dotcur::Song*)CurrentList->GetSongEntry(GetCursorIndex()))->Difficulties.size())
						DifficultyIndex = 0;
				}
			}
			return true;
		case KT_Select:
			Vec2 mpos = GameState::GetWindow()->GetRelativeMPos();
			if (!isMouseInput || mpos.x > Transform(mpos.y))
			{
				if (!CurrentList->IsDirectory(GetCursorIndex()))
					OnSongSelect(CurrentList->GetSongEntry(GetCursorIndex()), DifficultyIndex);
				else
					CurrentList = CurrentList->GetListEntry(GetCursorIndex());

				return true;
			}
		}

		switch (key)
		{
		case 'Q':
			PendingVerticalDisplacement += 320;
			return true;
		case 'W':
			PendingVerticalDisplacement -= 320;
			return true;
		case 'E':
			GoUp();
			return true;
		}
	}

	return false;
}

void SongWheel::GoUp()
{
	boost::mutex::scoped_lock lock (*mLoadMutex);

	if (CurrentList->HasParentDirectory())
		CurrentList = CurrentList->GetParentDirectory();
}

bool SongWheel::HandleScrollInput(const double dx, const double dy)
{
	PendingVerticalDisplacement += dy * 90;
	return true;
}

Game::Song* SongWheel::GetSelectedSong()
{
	int Index = GetCursorIndex();
	if (Index >= CurrentList->GetNumEntries())
		return NULL;

	if (CurrentList->IsDirectory(Index))
		return NULL;
	else return CurrentList->GetSongEntry(Index);
}

void SongWheel::Update(float Delta)
{
	uint32 Size;

	Time += Delta;
	SelCursor->Alpha = (sin(Time*6)+1)/4 + 0.5;

	if (!CurrentList)
		return;

	{
		boost::mutex::scoped_lock lock (*mLoadMutex);
		Size = CurrentList->GetNumEntries();
	}
	

	if (PendingVerticalDisplacement)
	{
		float ListDelta = PendingVerticalDisplacement * Delta * DisplacementSpeed;
		float NewListY = CurrentVerticalDisplacement + ListDelta;
		float NewLowerBound = NewListY + Size * ItemHeight;
		float LowerBound = Size * ItemHeight;

		{
			CurrentVerticalDisplacement = NewListY;
			PendingVerticalDisplacement -= ListDelta;
		}
	}

	Vec2 mpos = GameState::GetInstance().GetWindow()->GetRelativeMPos();

	if (mpos.x > Transform(mpos.y)) // change to InWheelBounds(mpos)
	{
		float posy = mpos.y;
		posy -= CurrentVerticalDisplacement;
		posy -= (int)posy % (int)ItemHeight;
		posy = (posy / ItemHeight);
		CursorPos = posy;
	}

	// Hey we've got a new cursor position, update.
	if (OldCursorPos != CursorPos)
	{
		OldCursorPos = CursorPos;
		DifficultyIndex = 0;
		if (OnSongChange)
		{
			Game::Song* Notify = GetSelectedSong();
			OnSongChange(Notify, DifficultyIndex);
		}
	}

	// Set its position.
	float Y = CursorPos * SelCursor->GetHeight() + CurrentVerticalDisplacement;
	float sinTSquare = sin(Time*2);

	sinTSquare *= sinTSquare;

	float X = Transform(Y) - SelCursor->GetWidth() -  sinTSquare * 10;
	SelCursor->SetPosition(X, Y);
}

void SongWheel::DisplayItem(String Text, Vec2 Position)
{
	if (Position.y > -ItemHeight && Position.y < ScreenHeight)
	{
		Item->SetPosition(Position);
		Item->Render();
		mTFont->Render(Text, Position + ItemTextOffset);
	}
}

void SongWheel::CalculateIndices()
{
	int maxItems = ScreenHeight / ItemHeight; // This is how many items we want to draw.

	// I only really need the top index.
	float V = floor(-CurrentVerticalDisplacement / ItemHeight);
	StartIndex = V;
	EndIndex = StartIndex + maxItems;
}

void SongWheel::Render()
{
	int Index = GetCursorIndex();
	boost::mutex::scoped_lock lock (*mLoadMutex);
	int Cur = 0;
	int Max = CurrentList->GetNumEntries();

	Game::Song *ToDisplay;

	CalculateIndices();

	for (Cur = StartIndex; Cur <= EndIndex; Cur++)
	{
		float yTransform = Cur*ItemHeight + CurrentVerticalDisplacement;
		float xTransform = Transform(yTransform);
		Vec2 Position = Vec2(xTransform, yTransform);

		if (Max)
		{
			int RealIndex = Cur % Max;

			while (RealIndex < 0) // Loop over..
				RealIndex += Max;

			if (!CurrentList->IsDirectory(RealIndex))
			{
				ToDisplay = CurrentList->GetSongEntry(RealIndex);
				DisplayItem(ToDisplay->SongName, Position);
			}
			else
			{
				DisplayItem(CurrentList->GetEntryTitle(RealIndex), Position);
			}
		}else
			DisplayItem("", Position);
	}

	if (!Max) return;
	// this below shouldn't be done here anyway..
	if (CurrentList->IsDirectory(Index))
	{
	}else
	{
		Vec2 InfoPosition = Vec2(ScreenWidth/6, 120);
		if (CurrentList->GetSongEntry(Index)->Mode == MODE_DOTCUR)
		{
			char infoStream[1024];

			dotcur::Song* Entry = static_cast<dotcur::Song*> (CurrentList->GetSongEntry(Index));
			if (Entry->Difficulties.size())
			{
				int Min = Entry->Difficulties[0]->Duration / 60;
				int Sec = (int)Entry->Difficulties[0]->Duration % 60;

				sprintf(infoStream, "song author: %s\n"
					"difficulties: %d\n"
					"duration: %d:%02d\n",
					Entry->SongAuthor.c_str(),
					Entry->Difficulties.size(),
					Min, Sec);

				mTFont->Render(String(infoStream), Vec2(ScreenWidth/6, 120));
			}else mTFont->Render(String("unavailable (edit only)"), InfoPosition);


		}
		else if (CurrentList->GetSongEntry(Index)->Mode == MODE_7K)
		{

			char infoStream[1024];

			VSRG::Song* Entry = static_cast<VSRG::Song*>(CurrentList->GetSongEntry(Index));

			if (DifficultyIndex < Entry->Difficulties.size())
			{
				int Min = Entry->Difficulties[DifficultyIndex]->Duration / 60;
				int Sec = (int)Entry->Difficulties[DifficultyIndex]->Duration % 60;
				float nps = Entry->Difficulties[DifficultyIndex]->TotalObjects / Entry->Difficulties[DifficultyIndex]->Duration;

				sprintf(infoStream, "song author: %s\n"
					"difficulties: %d\n"
					"duration: %d:%02d\n"
					"difficulty: %s (%d keys)\n"
					"avg. nps: %f\n",
					Entry->SongAuthor.c_str(),
					Entry->Difficulties.size(),
					Min, Sec,
					Entry->Difficulties[DifficultyIndex]->Name.c_str(), Entry->Difficulties[DifficultyIndex]->Channels,
					nps);

				mTFont->Render(String(infoStream), InfoPosition);
			}
		}

	}

	SelCursor->Render();

}
