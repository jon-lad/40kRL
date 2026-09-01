#pragma once

// Abstract base for every object that participates in save/load.
// The TCODZip archive is written and read in a strict order — every save must
// have a matching load that reads the same fields in the same sequence.
class Persistent {
public:
	virtual void save(TCODZip& zip) = 0;
	virtual void load(TCODZip& zip) = 0;
};