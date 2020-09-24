case $1 in
	compile)
		g++ accumSimulator.cpp -std=c++11 -o accumSimulatorOutput.out
		g++ -c accumMemory.cpp -std=c++11 -o accumMemoryOutput.out
		;;
	execute)
		./accumSimulatorOutput.out
		;;
	wipe)
		rm accumSimulatorOutput.out accumMemoryOutput.out
		;;
	*)
		echo command invalid or missing
		;;
esac
