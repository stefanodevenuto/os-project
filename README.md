# Chessboard Game

Multiplayer (CPUvsCPU) game, where the CPU that captures the highest number of flags wins!   
Project done during the "Operative Systems" course @ Università degli Studi di Torino (2019)

## Index
1. IPCs used
2. Sinchronization of the game
3. Player strategy-alghoritm for the Pawns
4. Movement strategy of the Pawns

# IPCs Used

### Semaphores
The Sinchronization of the entire Game is regulated with a series of Semaphores and Message Queues. The Semaphores used are:
1.  **Master Semaphores**
    * **Main Semaphore**
    This is used globally in all the three executable and it's the core of the entire Synchronization.
    It is composed by 5 entry.
    * **Chessboard Semaphore**
    This is the semaphore used to manage the positioning and the movement of the pawns.
    * **Turn Semaphore**
    Used to implement the mutual exclusion positioning for the player.
    Every entry represents a Player.
2. **Players Semaphores**
    An array of 4 semaphores used to garantuee the synchronization between Player and Pawns, usually in Wait-for-0 mode.

### Message Queues
1. **Master-Players queue**
    * Used to send the Taken-Flag messages and the Used Moves from the Players to the Master
2. **Player-Pawns**
    1. Used by the player to send the Positions and the Goal-Flag dispositions to the Pawns
    2. Used by the Pawns to send the Taken-Flag Messages to the Player, in order to not receive the       sent message while reading the Strategy Messages.

### Shared Memories
1. **Chessboard**
    The *chessboard* is realized with an array allocated in Shared Memory.
    The size of this portion of memory is the multiplication of the columns number and the rows number.

    > To treat it *like a matrix*, the solution is to applicate *row * columns_number + column* formula everytime is necessary to access the chessboard.

2. **Parameters**
    The parameters, read by the Master from the *config.txt* file, are stored in Shared Memory, in order to make them accessible to the Players and the Pawns.
3. **Positions**
    Used to store the calculated positions and make them accessible by the Players, in order to complete the positioning phase.

# Synchronization
The Synchronization is made up by different steps:
1. The Master initializes the SYNCHRO entry with 0 and the MASTER entry with the number of players.
   He also initialize the size-1 entry of the Turn Semaphore with 1, to make accessible the positioning for the first player. 
   After that, he forks the Players and wait for 0 on MASTER the check from the last ones.
2. The players set the first entry of the Players Semaphore with the number of pawns and try to access the positioning critical section. To do that, he tries to reserve the semaphore of the player before him and, if he did it, release his entry.
The mechanism is schematized in the image below:
<br>

![Critical Section](./img/Positioning.png)

In the critical section, a player peek a valid position in the chessboard by taking it in the Positions Shared Memory, every time it's his turn, and send that with the first Message Queue to the Pawns.
Then, he forks the pawns, and wait for 0 on the semaphore he set.

3. The Pawns read the message related to their type and set their positions, unlock the players by reserving the player set semaphore and wait in read for the strategy message.

4. The unlocked players unlock the master and wait on the preset SYNCHRO entry of the Main Semaphore

5. The master calculates the number of flags, their score and place them in the chessboard.
Then he set the A entry and the SYNCHRO entry of the Main Semaphore to the number of players and wait for 0 on the first one.

6. The player set again the first entry of the Player Semaphore, calculate the associations flag(s)-pawn, send the strategy to the pawns and set the third entry of the Player Semaphore to the number of pawns, to notificate them that the strategy is ready. 
Then he wait for 0 on the set semaphore.

7. The pawns unblocks theirselves by reading the message, handle the received strategy by creating an array to store all the informations, unlock the players and wait on the START entry of the Main Semaphore.

8. The players unlock the Master by reserving on A entry.

9. The game starts with the Master that unlock the players by setting the MASTER entry to the number of Players and the players unlock the Pawns by setting the START entry to yhe number of them.

10. Now that the game is started, the Master wait for a number of messages equal to the number of the flags set. If these messages doesn't arrive in time, the game ends.

11. The player wait on the second entry of the Player Semaphore array that all the pawns terminates their work.

12. The pawns use the strategy sent to try to get the associated flag. In case of success, the pawn notificates the player by sending a message with the score of the flag in the second messgae queue.
But in every case, the pawn check if she has anothe flag associated by reading the first queue.
After all the job is done, she unlock the Player and wait on the second entry of Player Semaphore.

13. The players read all the messages, reply those to the Master, unlock the pawn for read the new positions and the remaining moves for every pawn and wait on WAIT_END_ROUND entry.

14. The Master read all the messages and unlock the players.

14. The pawns send the informations to the players and the players send those to the Master, which updates the statistics of the Players.

15. The game **restart** from point 5.

# Player strategy-alghoritm for the Pawns

The idea behind the alghoritm is to find, for every flag, the nearest pawn of the squad.
That is in fact the first step, which is made with the pawn struct:

```c
struct pawn{
    long type;
    int pid;
    int x;
    int y;
    int starting_x;
    int starting_y;
    int remaining_moves;
    struct flag * target;
    struct flag * temp_target;
    int assigned;
    int temp_assigned;
};
```
For every flag and for every pawn, the distance between them is calculated, and if it's the lowest, the correspondent flag is assigned to the temp_target flags array and the temp_assigned variable is incremented, **but** the x variable and the y variable are not modified, in order to find all the possible assignable flags.
The flag struct is composed by:

```c
struct flag{
	int number;
	int position;
	int x;
	int y;
	int distance;
	int checked;
};
```
The second step consists in reorganize the temp_target array in order to obtain the nearest of the assigned flags. Now the position and the real target array of the pawn will be updated, the temp_target array and the temp_assigned variable cleared and the flag *checked*, in order to find new assignable flags in the first step.
Whenever a flag is assigned, a *strategy* is made, following this type of message struct:

```c
struct player_strategy{
    long mtype;
    int directions[4];
    int flag_x;
    int flag_y;
    int flag_position;
    int selected;
};
```
The directions array will contain the number of N/S/E/W moves to be done in order to obtain the flag.
This message will be sent in the first Player Message Queue, and later the Pawn will read that.
These two steps are repeated until every flag is checked or unreachable.

# Movement strategy of the Pawns

The pawn takes the Directions from the Message Queue from the player and the first thing that she do is check the selected variable is the message. If it's not, she directly wait to the second entry of the Player Semaphore, otherwhise, until she has moves, she checks if the assigned flag is not taken and tries to move in one of the directions assigned by the Player.

If the flag disappear or is taken by her, the Pawn try to read from the first Message Queue if there's another pawn assigned to her, calculates the new direction based on her position and repeat the loop. Otherwise, she wait the termination of the others Pawns in the second entry of the Player Semaphore. 