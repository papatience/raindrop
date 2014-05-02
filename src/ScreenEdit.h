#ifndef SCR_EDIT_H_
#define SCR_EDIT_H_

#include "ScreenGameplay.h"
#include "GuiTextPrompt.h"

class ScreenEdit : public ScreenGameplay
{

	enum 
	{
		Playing,
		Editing
	}EditScreenState;

	GUI::TextPrompt OffsetPrompt, BPMPrompt;
	uint32_t CurrentFraction;
	uint32_t CurrentTotalFraction; // basically beat snap
	uint32_t savedMeasure;
	BitmapFont EditInfo;

	GameObject* HeldObject;

	void IncreaseTotalFraction();
	void DecreaseTotalFraction();

	GameObject &GetObject();

	float YLock;
	enum
	{
		Select,
		Normal,
		Hold
	}Mode; 
	GraphObject2D GhostObject;

	bool  GridEnabled;
	int32 GridCellSize; // ScreenSize / GridCellSize
	// float GridOffset;

	void DecreaseCurrentFraction();
	void IncreaseCurrentFraction();
	void SaveChart();
	void SwitchPreviewMode();
	void InsertMeasure();

	void OnMousePress(KeyType tkey);
	void OnMouseRelease(KeyType tkey);

	void CalculateVerticalLock();
	void RunGhostObject();
	void DrawInformation();
public:
	ScreenEdit (IScreen * Parent);
	void Init(SongDC *Other);
	void StartPlaying(int32 _Measure);
	void HandleInput(int32 key, KeyEventType code, bool isMouseInput);
	bool Run (double Delta);
	void Cleanup();
};

#endif