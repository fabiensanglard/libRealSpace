# mission PROG chunk OPCODE LIST
- AB (171) set message indexe to waypoint for navmap
- A5 (165) assign spot to waypoint for navmap$


- A7 (167) set primary target for enemy ai, value is the id of the part (object) in the mission file


# NOTE on TEAM CHUNK
so it's seems that the team is define in the mission filed as follow:
- first ushort (16 bit) is the leader. the value is the index of the part in the mission file (beware, not the actual id, but the position in the part array)