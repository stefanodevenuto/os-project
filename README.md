# OS-Project 2019/2020
## Index
1. IPC's used, sinchronization of the game and related Scheme
2. Positioning of the Pawns
3. Player strategy-alghoritm for the Pawns
4. Movement strategy of the Pawns

# IPC's Used

## Semaphores
The Sinchronization of the entire Game is regulated with a series of Semaphores and Message Queues. The Semaphores used are:
1.  **Master Semaphores**
    * **Main Semaphore**
    This is used globally in all the three executable and it's the core of the entire Synchronization.
    It is composed by 5 entry.
    * **Chessboard Semaphore**
    This is the semaphore used to manage the positioning and the movement of the pawns.
    * **Turn Semaphore**
    Used to implement the mutual exclusion positioning for the player.
2. **Players Semaphores**
    An array of 4 semaphores used to garantuee the synchronization between Player and Pawns, usually in Wait-for-0 mode.

## Message Queues
1. **Master-Players queue**
    Used to send the Taken-Flag messages and the Used Moves from the Players to the Master
2. **Player-Pawns**
    1. Used by the player to send the Positions and the Goal-Flag dispositions to the Pawns
    2. Used by the Pawns to send the Taken-Flag Messages to the Player, in order to not receive the sent message while reading the Strategy Messages.

## Shared Memory
1. **Chessboard**
    The *chessboard* is realized with an array allocated in Shared Memory.
    The size of this portion of memory is the multiplication of the columns number and the rows number.

    > To treat it *like a matrix*, the solution is to applicate *row * columns_number + column* everytime the access is necessary.