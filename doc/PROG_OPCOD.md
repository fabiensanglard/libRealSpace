# mission PROG chunk OPCODE LIST
- AB (171) set message indexe to waypoint for navmap
- A5 (165) set objective to fly to waypoint
- A6 (166) set objective to fly to way area
- A2 (162) set objective to land plane at waypoint
- A1 (161) set objective to takeoff from waypoint (simulate takeoff for ai)
- AA (170) set objective to follow target with id as the value of the index of the part in the mission file
- A8 (168) set objective to defend ally with id as the value  of the index of the part in the mission file
- A7 (167) destroy target with id as the value id is the index of the part in the mission file
- 92 (146) check if part ID is in same area.
- 94 (148) destroy / instant kill target
- BE (190) deactivate target / disable AI on target id
- 48 (72) execute ai state
- 08 (8) define ai state
- 01 (1) exit prog

# not complety sure about the following

- 45 (69) check flag with id as flag number
- 10 (16) set label number



- 52 (82) set flag with id as flag number

From the strike.exe we should have the following:

- 161 : Take off 
- 162 : Landing  
- Fly to 
Precise Way
- 165 : Fly to 
Way Point
- 166 : Fly to 
Way Area
- 167 : Destroy
Target
- 168 Defend
Ally
- Defend
Point
- 170 Follow
Leader

# NOTE on TEAM CHUNK
so it's seems that the team is define in the mission filed as follow:
- first ushort (16 bit) is the leader. the value is the index of the part in the mission file (beware, not the actual id, but the position in the part array)