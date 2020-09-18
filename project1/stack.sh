case $1 in
	compile)
		g++ stackSimulator.cpp -std=c++11 -o stackSimulatorOutput.out
		g++ -c stackMemory.cpp -std=c++11 -o stackMemoryOutput.out
		;;
	run)
		./stackSimulatorOutput.out
		;;
	wipe)
		rm stackSimulatorOutput.out stackMemoryOutput.out
		;;
	*)
		echo command invalid or missing
		;;
esac
