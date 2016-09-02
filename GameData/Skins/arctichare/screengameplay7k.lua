-- 7+1 and 5+1 supported only
if Channels ~= 8 and Channels ~= 6 then
	fallback_require "screengameplay7k"
	return
end

game_require "TextureAtlas"
FixedObjects = game_require "FixedObjects"

game_require "utils"
game_require "AnimationFunctions"

-- Set up constants for everyone

skin_require "skin_defs"
skin_require "custom_defs"
Components = skin_require "components"

-- All of these will be loaded in the loading screen instead of
-- in the main thread, and will also be unloaded at the end.
skin_require "preload"

-- Status of a lane being pressed or not.
KeyArray = {}

-- Lightning
Lightning = {}
LightingTime = 0.25

Bomb = {}
BombTime = 0.2

function Init()
	Objs = FixedObjects:new()
	Objs.XRatio = SkinScale
	Objs.YRatio = SkinScale
	AutoadjustBackground({
		x = 880 * SkinScale,
		y = 200 * SkinScale,
		w = 2080 * SkinScale,
		h = 1170 * SkinScale
	})

	print("Initializing")
	Components:Init()
	IsFullCombo = false

	print ("Create fixed objects")
	Objs:CreateFromCSV("hare.csv", Noteskin[Channels])
	ObjP1 = FixedObjects:new()
	ObjP1.XRatio = SkinScale
	ObjP1.YRatio = SkinScale
	ObjP2 = FixedObjects:new()
	ObjP2.XRatio = SkinScale
	ObjP2.YRatio = SkinScale
	if Channels == 6 then
		local p1s = table.join(Channels6P1, Channels6Common)
		local p2s = table.join(Channels6P2, Channels6Common)
		table.dump(p1s)
		table.dump(p2s)
		ObjP1:CreateFromCSV("5k.csv", p1s)
		ObjP2:CreateFromCSV("5k.csv", p2s)
	else
		Objs:CreateFromCSV("7k.csv", Channels8P2)
		-- FixedObjects.CreateFromCSV("7k.csv", Channels8P1[Channels])
	end

	for i=1, Channels do
		KeyArray[i] = 0
	end
end

function Cleanup()
end

function OnFullComboEvent()
end

function OnFailureEvent()
	if Global.CurrentGaugeType ~= LT_GROOVE then
		DoFailAnimation()
		return FailAnimation.Duration
	else
		FadeToBlack()
		return SuccessAnimation.Duration
	end
end

-- When 'enter' is pressed and the game starts, this function is called.
function OnActivateEvent()
end

function HitEvent(JudgmentValue, TimeOff, Lane, IsHold, IsHoldRelease)
    Components:OnHit(JudgmentValue, TimeOff)
end

function MissEvent(TimeOff, Lane, IsHold)
	Components:OnMiss(5, TimeOff)
end

function KeyEvent(Key, Code, IsMouseInput)
end

function GearKeyEvent (Lane, IsKeyDown)
    KeyArray[Lane + 1] = IsKeyDown
end

-- Called when the song is over.
function OnSongFinishedEvent()
	DoSuccessAnimation()
	return SuccessAnimation.Duration
end

function Update(Delta)
	-- Executed every frame.
	local beatEffect = Beat - math.floor(Beat)

	local SongPercentage = Game:GetSongTime() / (SongDuration + 3)

	if Game:GetSongTime() < 0 then
		SongPercentage = math.pow(SongTime / -1.5, 2)
	end

	Components:Update(Delta)
end
