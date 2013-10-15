_ENV=module(...)

function click(sender)
	local name = sender:getName()
	if name == "sphere" then
		local sphere = geo.createRigidSphere(50,true)
		sphere:translate(0,0,0)
		sphere:setMaterialName("Ogre/Tusks")
	elseif name=="run" then
		brun = not brun
		geo.simulation(brun)
		local text
		if brun then
			text = "Pause"
		else
			text = "Run"
		end
		run:setCaption( text )
		collectgarbage("collect")
	end
end

function openStage()
	gui.loadResource("MyGUI_BlackOrangeTheme.xml")
	game.setCameraControlet("simple")
	local w = gui.loadLayout("testRigid.layout")
	if w then
		local sb = w:child("sphere")
		sb:setScript("eventMouseButtonClick",click)
		run = w:child("run")
		run:setScript("eventMouseButtonClick",click)
		brun = false
	end
end

function closeStage()
end

game.addStage("testRigid",openStage,closeStage)