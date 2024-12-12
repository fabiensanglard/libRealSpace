# mission PROG chunk OPCODE LIST
- AB (171) set message indexe to waypoint for navmap
- A5 (165) set objective to fly to waypoint
- A6 (166) set objective to fly to way area
- A2 (162) set objective to land plane at waypoint
- A1 (161) set objective to takeoff from waypoint (simulate takeoff for ai)
- AA (170) set objective to follow leader
- A8 (168) set objective to defend ally with id as the value

- A7 (167) set primary target for enemy ai, value is the id of the part (object) in the mission file

# not complety sure about the following

- 45 (69) set ai state machine id (MVRS in RSPROF)
- 10 (16) set label number
- 48 (72) set label number
- 08 (8) goto label number
- 01 (1) if objectiv is complete, skip next instruction
- 92 (146) check if part id is alive


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