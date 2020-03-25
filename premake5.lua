tests = {"tests/parse.c"}

function gen_tests()
	for k, v in pairs(tests) do
		b, e = v:find("/[a-z]+");
		b=b+1;
		name = v:sub(b,e);
		print ("generating test", name)
		project ("test-" .. name)
			kind "ConsoleApp"
			language "C"
			targetdir "bin"
			debugdir "tests"

			includedirs "./"
			links ("json")
			links "m"
			files (v)

			filter "configurations:Debug"
				defines { "DEBUG=1", "RELEASE=0" }
				optimize "off"
				symbols "on"
	
			 filter "configurations:Release"
				defines { "DEBUG=0", "RELEASE=1" }
				optimize "on"
				symbols "off"

			buildoptions "-Wall"
	end
end

newoption {
	trigger = "test",
	description = "Build the tests",
}

workspace "jsonparser"
	configurations { "Release", "Debug" }

v = "tests/parse.c"

if _OPTIONS["test"] then  gen_tests() end

project "json"
	kind "StaticLib"
	language "C"
	targetdir "bin"
	debugdir "tests"

	links "m"
	
	files { "src/**.h", "src/**.c" }

	filter "configurations:Debug"
		defines { "DEBUG=1", "RELEASE=0" }
		optimize "off"
		symbols "on"

 	filter "configurations:Release"
		defines { "DEBUG=0", "RELEASE=1" }
		optimize "on"
		symbols "off"

	buildoptions "-Wall"
