Ryan Zesch
Final Project - 3D Slay the Spire

To build, make a build directory, run 'cmake ..' in that directory, make, and enjoy.
After building, run with ./3DSlayTheSpire

Controls:
    W/S    - Dolly camera
    A/D    - Strafe camera
    Mouse  - Change view direction - should be continuous, mouse invisible + locked at center of window
    LShift - Sprint while moving (Also changes FOV)
    Q/E    - Select a card to the left/right (Moves selector above cards)
    F      - Throw selected card
    R      - Draw a new hand of cards (when refresh = 0)
    G      - Turn on "God Mode" - 9999 energy, health, and block
    N      - Turn on noclip (unbind from a horizontal plane)

This is a 3D reimagining of the turn based card game Slay the Spire.

Camera locked to xz plane. Game is played by throwing cards at enemies. "Strike" cards are thrown and deal damage.
"Block" cards increase the amound of shield you have. Each time you draw a handof cards, you get 3 energy. You can
draw a new hand each time the refresh time is 0.

Text is rendered to the screen using the freetype library.