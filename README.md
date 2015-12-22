3D Pong


![Splitscreen with facing paddles](https://raw.githubusercontent.com/DanielMiles/3D-Pong/master/Screenshot-1.png "Screenshot")

What we have done:

- Ported the hw04 shader to hw03 and used it to create lighting effects on the inside of our cube for both the paddles and the sphere, using the fragment shader we treat the paddles and the sphere as light sources to get some really good looking (if computationally intensive) effects. 
- Modified the keyboard control system to operate in the render() function, this allows significantly better response than the existing keyboard input function and makes the game actually playable if you can get over the 3D thinking. 
- Split viewport, or in reality really rapidly changing viewpoints, in the name of not duplicating the entire rendering pipline we use a couple of view ports and a special function to update the eye position, combined with the use of a special opengl call to keep one view port from being blanked while we swap the buffers this creates a very smooth dual view effect so long as the framerate remains high enough (works great on lab computers, not so well on my laptop at home). 
- Paddle deformation, when the ball impacts a paddle it causes a square or rectangular block to be pushed out and backwards, this block gets smaller as the ball hits points closer to the edges. 
- For split viewport's the eye for each view port has been attached to an opposing paddle, this makes the game much more easily playable than fixed viewports on the other side or worse viewports on your own side. 


How to play:

- make ; ./hw03 (Dependencies include Nvidia drivers and imagemagick) 
- Find a friend, or play against yourself 
- use wasd to control one view port and 8456 on the numpad (with numlock on!) to control the other. 
- To make the game fun as time goes on the balls velocity will increase slowly, also the ball will tend to trend in a axis until it hits a wall so the players will have to follow it as it moves, actual random deflection requires too much movement to be easily playable. 

Other things of note: 

- If splitscreen looks horrible try it on one of the lab computers, the trick I used to get dual viewports without duplicating the rest of the rendering pipline is very dependent on the framerate being solid to look nice. 
- to explore our construction (and better appreciate deformation, its hard to do so from a head on view point) toggle splitscreen_on in init() off and recompile, this will allow you to fly around the world unimpeded with a single view port. 
