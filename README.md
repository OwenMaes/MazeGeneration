# Maze generation: Magic Maze
---

What is a maze?
A maze is a collection paths that most of the time has a starting entrance and an end goal. It's a very well-known puzzle or tourist attraction. It originates from the famous Greek mythology, where maze is built to hold the minotaur. Even its creator had a difficult time leaving the structure after it was built. Mazes can be found in a lot of media, from movies to games. In this Unreal C++ project, I wanted to make a maze that changes every x-amount of seconds, indicated by crumbling walls.

### Maze generation algorithms 
There are a lot of algorithms that can generate a maze; however, I choose to only implement three of many. By searching about the subject, I came across these three algorithms the most and that's the random depth-first search, random Kruskal's and random Prism's algorithm.

#### Random depth-first search
Also known as the "recursive backtracker" and like the others is the randomized version of the algorithm. You start with a grid of cells with connections to its adjacent cells. You pick a starting point in your grid of cells and randomly pick a direction. You visit the cell in that direction and mark it visited. You backtrack to the previous cell if all the directions point to a visited cell. Doing this recursively will result in a carved-out maze.
![Maze](https://user-images.githubusercontent.com/97401433/195191679-569108ad-9544-4b39-bc85-848ad2be07e0.png)

##### Random DFS implemation problems
Going above a maze cell grid of 87 rows and 87 columns resulted in a stack overflow error. When using this method on a thread it even limits the grid more (25 x 25). A common stack overflow cause is very deep or infinite recursion because the function calls itself too much and with function comes the parameters and return address. This probably is the cause of the running out of stack space. The other algorithms can generate way bigger grids (200x200).

### Mesh generation
I use the "Instanced Static Mesh" component in Unreal Engine 4 to quickly generate different instances of the same mesh. This component holds a static mesh and a material, it only needs a transform to create a new instance. I use three ISM components, one for the outer walls. The outer walls don't change unless you change the width or height of the maze dimensions. ![OuterWalls](https://user-images.githubusercontent.com/97401433/195194595-028f0618-2d97-4937-a24e-d0bfe5070eca.png)
The same goes for second component, which is used to instantiate the floors.
![Floors](https://user-images.githubusercontent.com/97401433/195194627-dcdbb75f-ca77-46a2-9f4a-620b5049b6b2.png)
The third component is used for the inner walls, these change every x-amount of seconds to change the mazes' layout.
![InnerWalls](https://user-images.githubusercontent.com/97401433/195194835-b84642b5-4b68-4d7b-aca3-ab7bd37278f7.png)

### Crumbling fx
I use Niagara for the crumbling effect. The crumbling effect is to indicate the difference between the old and the new inner walls. I have an Array that stores the connections between the nodes (walls). This is done before the wall Array is given to the maze generation algorithm as a parameter which returns the new connections (walls) of the maze. I check which of these connections of the old array were walls but are openings in the new array, these positions are used to spawn the erosion Niagara systems.![HighresScreenshot00001](https://user-images.githubusercontent.com/97401433/195196589-a1282dd5-7f6f-4299-ac74-9c96d6c2e3da.png)

##### Spawning Niagara Systems problems
Spawning a lot of Niagara systems causes lag. Even though all the particles are on the gpu, there all still 1000 particles for every crumbling fx Niagara system. An idea to mitigate this lag is to only spawn these crumbling effects around the player in radius that is set.


### References
-	Maze generation algorithm. (2022, October 8). In Wikipedia. https://en.wikipedia.org/wiki/Maze_generation_algorithm
-	Pullen , W. D. (2022). Maze Classification. astrolog. http://www.astrolog.org/labyrnth/algrithm.htm#perfect
-	Buck, J. (2015). Mazes for Programmers: Code Your Own Twisty Little Passages. Pragmatic Bookshelf.
-	Maze. (2022, October 8). In Wikipedia. https://en.wikipedia.org/wiki/Maze
