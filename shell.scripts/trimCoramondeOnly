#!/bin/bash
# This script deletes all sectors except the 3x3x3 boxes of sectors centered at:
#   Coramonde Sector         (3115,5,5)
psql --command 'DELETE FROM a_Sector WHERE id NOT IN (SELECT id FROM a_Sector WHERE (locx >= 3105 and locx <= 3125 and locy >= -5 and locy <= 15 and locz >= -5 and locz <= 15));' auUniverse -U aumaster
