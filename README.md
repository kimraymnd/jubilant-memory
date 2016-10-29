# jubilant-memory


####Background and Assumptions

For the sake of simplicity, I let the fat polyhedra be as simple as they could be: cubes. In the ouset, I also make a pretty big assumption that our polyhedrons were regular, which make some of the calculations a lot easier to deal with. I start by generating a sequence of starting positions, since I wanted to have an initial state where there were no collisions. Next, I give each object a random size, velocity, rotational axis and rotational velocity. For the size, I assumed lower bounds and upper bounds of 1 and 4 since I wanted the cubes to reasonably fit within the overall boundaries. I also put bounds on the velocities as I didn't want the cubes to spin too fast.  The polyhedras were set up in a 15 x 15 x 15 cube.

There is a continuous while loop that renders each frame until the 'esc' key is hit and for each frame, we sweep the scene by using Point Swept Spheres; calculating the spheres were easy because of fact that the polyhedras I were working with were Platonic solids. If the sweep detects two solids whose distance were less than that of their size, we do another more exact collision check in which we determine whether or not the solids actually collide. If a collision is detected, we respond by negating the angular velocity and position velocity (realistically, this is completely wrong; determining the correct response becomes a non-trivial problem and would have taken a while to compute). Although non-realistic, this response is sufficient as it guarantees that a collision state will not continuously remain. 

The cubes can be easily extended to other Platonic Solids, such as octohedrons or icosahedrons. The difference will come in the VAOs and VBOs we use the render each frame and also the method that is called after we do our first sweep. Other than that, the program should remain the same. 

I tried to use the V-Collide library and got it working at one point (in a test project) but it wasn't working with the libraries that I were using in my current project. It would be a neat project to work on V-Collide to get it up to date with some of the current standards. 

Here are some screencaps of what you would expect to see (some look like they are collding, but they are not)

Ten Cubes with varying sizes:
![Alt text](/images/ten.png?raw=true "Ten Cubes with varying sizes")
Thirty Cubes with varying sizes:
![Alt text](/images/thirty.png?raw=true "Thirty Cubes with varying sizes")
Fifty Cubes with varying sizes:
![Alt text](/images/fifty.png?raw=true "Fifty Cubes with varying sizes")


####Analysis 

Using glfwGetTime(), I took the difference of the return values before and after I do my collision check and summed it over 1000 iterations, giving me the following values: 

| Number of Objects   | Performance | 
|:----:|:---------:|
| 10 | 0.126256 |
| 20 | 0.202029 |
| 30 | 0.449823 |
| 40 | 0.809079 |
| 50 | 1.249240 |

This was somewhat expected as the sweepings only take O(n^2) runtime and given the polyhedron the second collision test takes a constant amount of time. The sweepings were also efficient in that we can already generate a bounding sphere due the assumption that our polyhedrons were regular. If we were dealing with non regular polyhedrons, then there would be an overhead in calculating the bounding point sphere for each object.

2. Complexity of objects (The number of polygons for each object.  Try at least 3 different data sets)

If we changed the complexity of the objects, the main overhead will come from doing our second round collision testing. The brute force method used it checking whether or not each edge will intersect with any of the faces. In doing so, we use polyhedra of 8, 12, 20 faces to get the following data (assume 25 objects in the scene):

|Number of faces| Performance |
|:--:|:--:|
|8|1.20709|
|12|2.38503|
|20|5.51971|

We notice some type of quadratic behavior since on average for 1 trangle face, we have 3 edges for that face and since two triangles share an edge, we'll have 1.5 times as many edge as we do faces. Since this is constant and the brute force collision test where we compare every edge to every face will be O(m^2) for m faces. 

3. Sizes of objects (The relative size of each object w.r.t. the extension of the bounding cube.)

In the 15 x 15 x 15 cube, I now generate 15 cubes with the following size range: [1,5], [3,7], [5,9]. This will take my scene to be set up a lot slower (with my implementation) and should cause more collisions to occur, causing more overhead. I generate the following data:

|Range of Size| Performance |
|:--:|:--:|
|[1,5]|0.433911|
|[2,6]|0.488744|
|[3,7]|0.524924|

We notice an increase, but the subtle difference probably arose from the fact that only 15 cubes were generated. I couldn't do any more (for all ranges) because it was taking too long to set the scene up.

####Varying Parameters

> Objects now have the same size. What would you do differently? Show a simple prototype implementation to compare and contrast the performance of two different algorithms (implementation).

I wouldn't change my implementation much (if any) since my assumptions were pretty friendly to size changes. I was dealing with analytical polyhedra which made intersection calculations fairly easy to deal with. If I had to deal with polyhedra of various complexities where I didn't know the center of mass and other attributes, then I'd likely use a BVH-based collision detection method (perhaps with OBBs). I haven't gotten an implementation for this yet, but its something I'll continue to work on.


Future Plans

- accommodate much more complex objects other than platonic solids
- integrate in V-Collide if I can manage to update it to current standards
- find the correct collision response to realistically simulate a physical collision rather than inverting the velocity vectors
