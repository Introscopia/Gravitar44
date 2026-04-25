
### SVG protocol:
 
 # Animations
  > are defined as a <g> group containing
   * an optional 'dope_sheet' attribute, which should be a comma-separated list of {curly-brace-enclosed} lists of comma-separated integers, i.e: "{0,1,2,3,4},{0,1,2,3,4,5,6,7,8},{0,5,6,7,8}"
   Each {curly-brace-enclosed} list describes a frame of the animation in order, the numbers being indices into the animation's "cells". If the dope sheet is omitted, the cells are interpreted as frames to be played in order (equivalent to "{0},{1},{2},{3},...").
   * a 'period' attribute, and integer defining the duration of each frame in milliseconds.
   * Any number of geos inside. (The "cells")
     A cell's id should end with its index: id="Rray_03"

 # Doodads
   > group all the elements for each doodad.
   > The group id will be the name of the doodad in-game.
  # Attributes:
   * class
     > "ship"
       * density
       * friction
       * thrust
       * turn_speed
       * fuel_max
       * fuel_consumption -- per second
  # Elements:
    > A Doodad's component svg elements' ids should be formatted ="<parent_Doodad_id>:<Element>", 
      i.e: "Hero:visual". This might seem annoying seeing as we already named it in the group, but!
      all svg elements need unique ids anyways, even across group boundaries, so I standardized it this way.
   * visual -- what you see onscreen. may be a group or a single geo.
   * exhaust -- What you see onscreen when the ship is thrusting. Should be an animation.
   * physical -- actual physics geometry. Convex shapes only!
   * center -- should be a <circle>. don't worry, the center of mass in handled automatically.
   * smoke -- should be a <circle>. where smoke particles spawn when the ship is thrusting.
  Ships

# MAPS
  > Layers:
   - Zones
     - <bounds> geo with this id will determine the bounds of the map.
       A rect will designate this as a FLAT map: gravity points down.
       A circle will designate this as a ROUND map: gravity points towards the origin.
       > attributes:
         - <camera>: can be set to "FIXED", which disables rotating camera in round maps.
     - <gravity_falloff>
     - <spawn>: area where ship will appear upon entering the map.
   - Visual
   - Physical
     > attributes:
       - <gravity>: numerical value, strength of gravity. note that values differ hugely between round and flat maps.
   - Holo

## Glossary
 * "geo" in an svg: a <path>, a <circle> or a <rect>.
 	<path>s may not have subpaths!!

## General notes
 * Everything is case sensitive.