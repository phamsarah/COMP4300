case $1 in
	compile)
		g++ accumSimulator.cpp -std=c++11 -o accumSimulator.out
		g++ -c accumMemory.cpp -std=c++11 -o accumMemory.out
		;;
	run)
		./accumSimulator.out
		;;
	wipe)
		rm accumSimulator.out accumMemory.out
		;;
	*)
		echo command invalid or missing
		;;
esac
