_ENV=module(...)

local dialog
local combo

function eventComboAccept(sender,index)
	if index == 0 then
	--	gui.loadResource("MyGUI_BlueWhiteTheme.xml")
		dialog.destroy()
	--	openStage()
	elseif index == 1 then
	--	gui.loadResource("MyGUI_BlackBlueTheme.xml")
		dialog.destroy()
	--	openStage()
	end
end

function openStage()
	print("Open stage")
	dialog = gui.loadLayout("testSkin.layout")
	combo = dialog:child("skin")
	combo:addItem("Blue")
	combo:addItem("Black")
	combo:setScript("eventComboAccept",eventComboAccept)
end

function closeStage()
	print( "Close stage" )
end

game.addStage("testskin",{["openStage"]=openStage,["closeStage"]=closeStage})