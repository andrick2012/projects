@Author : Andrick Adhikari and Zibo Zhou

@Overall state: Program is complete and succesfully implements all the requirements stated in the assignment.

@Usage: ./loader is used to load the routers, Distance vectors are updated untill whole system stablizes.

		For ease of testing: three root graphs are included : neighbors, neighbors2 and neighbors3 which points to graphs given in the assignment:
															  neighbors is for 3 router system. : ./loader 3 neighbors
															  neighbors2 is for 5 router system.: ./loader 5 neighbors
															  neighbors3 is for 6 router system. : ./loader 6 neighbors

															  The code will terminate if router numbers provided in the argument doesnt match the number of graph files
															  and graph information.

	  ./tester is for testing a data packet: ./tester <source router number> <destination router>


@log messages:

	When the program initializes,
	all the router tables will be printed.

	Whenever a packet is recieved by the router: log message specifying the type of packet will be printed.
	If a packet is forwarded: log message will be printed.
	Whenever control packet leads to update in table: new table for the router will be printed.
	When data and control packet reaches the intended router, log message will be printed.

@Termination

./loader needs to be terminated with CTRL + C.
./tester terminates as soon as it pushes the packet to starting router.
