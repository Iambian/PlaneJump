Plane Jump
==========

Warning
-------
* This game works on a **TI-84 Plus CE**
* This game **will not** work on a TI-84 Plus (SE). Go play
	[this](https://www.ticalc.org/archives/files/fileinfo/96/9687.html)
	if you have that calculator.
* This game **will not** work on a TI-84 CSE. If you have that calculator,
	you're out of luck.

About
-----
When I was in high school, I enjoyed playing
[Plain Jump](https://www.ticalc.org/archives/files/fileinfo/96/9687.html)
on my TI-83 Plus while I was bored out of my mind in math class (or any other
class I could get away with). There was just something enthralling about the
perspective and its buttery-smooth graphics, so when I remembered it while I
was pondering on what to write next for the TI-84 Plus CE, I knew I had to
recreate that experience.

Did that work out? Let me know!

![Animated screenshot of use and gameplay](/_sshot.png?raw=true)

How To Play
-----------
1. Find the `PJUMP.8xp` file in the `bin/` folder.
2. Use your favorite link program to send it to your 84 CE, or an
	appropriate emulator
3. Run PJUMP (Plane Jump) from your favorite shell/launcher, such as Cesium.
	This is probably the best/easiest way to do it.
4. If you don't have that, you can run PJUMP on the homescreen by clearing the 
	homescreen, and then do the following:
	*	Clear the homescreen
	*	Paste `Asm(` from the catalog
	*	Paste the program name from the program menu
	*	See that the line reads `Asm(prgmPJUMP` and press ENTER
5. If you tried steps 3 or 4 and PJUMP tells you that you need additional
	files to run the game or if it tells you that what you have are outdated,
	follow the instructions it gives you and try again. You can download the
	[latest libraries](https://github.com/CE-Programming/libraries/releases)
	and send those to your calculator to try to preempt that.

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
* Andreas Ess - For authoring the game this is based on.
* Tim - For the coffee, sanity, and rigorous playtesting
* Xeda - Suggestions on how to implement jump effect
* Eeems - Suggestions on ball landing effect
* kg583 - For pointing out that there needs to be a delay between death and game over.
	and for initial hardware testing
* fghsgh - I don't really recall what for, but remembering that name gives me
	warm and fuzzy feelings for some reason
* Google - for image searching and that cool explosion sprite that I cribbed off
	of it and re-used for nearly all my game projects (where it would make sense. -ish.)
* CodeBros, Omnimaga, and Cemetech chat/Discord groups

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

Screensaver/Eyecandy Mode
-------------------------
On the title screen with the `Start Game` option selected, push 
the [Left], [Right], and [2nd] buttons at exactly the same time.
If successful, you won't be able to lose the game. Neither will the
score increase, but that's okay if you're doing it to stare at the
pretty pictures, or figure out any slight imperfections in the graphics.

Known Problems
--------------
* No proper level support - I tried. But I got lazy and left it with a
	a random level generator after the first few static segments.
* Reports of minor screen tearing and slight slowdowns - not sure how to fix.













