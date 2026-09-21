#!/bin/bash
# This script deletes all sectors except the 3x3x3 boxes of sectors centered at:
#   Hazeron Sector         (245,-885,-15)
psql --command 'DELETE FROM a_Sector WHERE id NOT IN (SELECT id FROM a_Sector WHERE (locx >= 1650-10 and locx <= 1650+10 and locy >= 10-10 and locy <= 10+10 and locz >= 10-10 and locz <= 10+10));' auUniverse -U aumaster
