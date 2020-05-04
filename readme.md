Plane Jump
==========

An implementation of the the game 
[Plain Jump](https://www.ticalc.org/archives/files/fileinfo/96/9687.html)
by Andreas Ess. This is a re-imagining/re-writing of that fun game that
I played so long ago while bored in math class. To that end, I tried to make
everything as buttery-smooth as possible. Let me know how that turned out.

How To Play
-----------
* Find the `PJUMP.8xp` file in the `bin/` folder.
* Use your favorite link program to send it to your 84 CE, or an
	appropriate emulator
* Run on the homescreen by clearing the homescreen, do the following
	*	Clear the homescreen
	*	Paste `Asm(` from the catalog
	*	Paste the program name from the program menu
	*	See that the line reads `Asm(prgmPJUMP` and press ENTER
* Or you can run the program from your favorite shell, such as Cesium.
	It's probably better to do it that way.

Controls
--------

Controls on the main menu

| Keys    | Function
|--------:|:-------------------
|[Mode]   | Ends the program
|[2nd]    | Selects the option colored in gold
|[Up/Down]| Moves the selection cursor up or down

Controls during gameplay

| Keys       | Function
|-----------:|:-------------------
|[Mode]      | Return to main menu
|[2nd]       | Makes the ball jump
|[Left/Right]| Moves the ball left or right
|[Up]        | Rolls the ball forward



Acknowledgements
----------------
* me.
* That acknowledgement at the very top of the readme file
* CodeBros, Omnimaga, and Cemetech chat/Discord groups
* Tim - For the coffee and sanity
* Xeda - Suggestions on how to implement jump effect
* Eeems - Suggestions on ball landing effect
* kg583 - For pointing out that there needs to be a delay between death and game over.
	and for initial hardware testing
* fghsgh - I don't really recall what for, but remembering that name gives me
	warm and fuzzy feelings for some reason
* Google - for image searching and that cool explosion sprite that I cribbed off
	of it and re-used for nearly all my game projects (where it would make sense. -ish.)

Building the Game From Source
-----------------------------

Make sure you have the latest version (or thereabouts) installed,
which can be found 
[here](https://github.com/CE-Programming/toolchain/releases) .
You may need to download the next-to-latest version and install that too if
the build process fails due to missing things.

Open a command-line console in the project's root directory and run the
following commands:

* `make gfx`
* `make`

If there were no errors, `PJUMP.8xp` will now be found in the `bin/` folder.


















