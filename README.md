# OS-Project 2019/2020
## Index
1. Sinchronization of the game and related Scheme
2. Positioning of the Pawns
3. Player strategy-alghoritm for the Pawns
4. Movement strategy of the Pawns

# Sinchronization of the game
The Sinchronization of the entire Game is regulated with a series of Semaphores and Message Queues. The Semaphores used are:
1. #### Master Semaphores
    * ##### Main Semaphore
    This is used globally in all the three executable and it's the core of the entire Synchronization.
    It is composed by 5 entry.
    * ##### Chessboard Semaphore
    This is the 
	
