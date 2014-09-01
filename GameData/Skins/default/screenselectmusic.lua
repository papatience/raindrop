-- Wheel transformation
TransformX =  ScreenWidth * 13/20
CurrentTX = ScreenWidth

-- List transformation
function TransformList(Y)
  return ((Y*Y - 768*Y) / (-1055)) + CurrentTX
end

function InBanner(frac)
	Obj.SetPosition( 0, -192 * (1-frac) )
	return 1
end

function InUpBtn(frac)
	Obj.SetPosition( ScreenWidth / 2, ScreenHeight + 60 - 81.5 * frac )
	return 1
end

function InBackground(frac)
	Obj.SetAlpha(frac)
	return 1
end

-- Screen Events
function OnSelect()

end

function OnRestore()
	CurrentTX = ScreenWidth	
	BgAlpha = 0
	Obj.SetTarget(DirUpButton)
	Obj.AddAnimation( "InUpBtn", 0.5, 0, EaseOut )

	Obj.SetTarget(Banner)
	Obj.AddAnimation( "InBanner", 0.5, 0, EaseOut )

	Obj.SetTarget(ScreenBackground)
	Obj.AddAnimation( "InBackground", 0.5, 0, EaseOut )
end


-- ButtonEvents
function DirUpBtnClick()
	CurrentTX = ScreenWidth
end

function KeyEvent(k, c, m)

end

function DirUpBtnHover()
	Obj.SetTarget(DirUpButton)
	Obj.SetImageSkin("up_h.png")
end

function DirUpBtnHoverLeave()
	Obj.SetTarget(DirUpButton)
	Obj.SetImageSkin("up.png")
end

function BackBtnClick()

end

function BackBtnHover()

end

function BackBtnHoverLeave()

end

function Init()
	Banner = Obj.CreateTarget()
	Obj.SetTarget(Banner)
	Obj.SetImageSkin("ssbanner.png")
	Obj.SetZ(18)

	Obj.SetTarget(DirUpButton)
	Obj.SetImageSkin("up.png")
	Obj.SetCentered(1)
	Obj.SetZ(18)
end

function Cleanup()
	Obj.CleanTarget(Banner)
end

function Update(Delta)
	CurrentTX = CurrentTX + (TransformX - CurrentTX) * Delta * 8
end