#!/bin/bash
# This script deletes all non-person atoms who do not have a parent atom.
psql --command 'DELETE FROM a_Atom WHERE idp > 0 AND id NOT IN (SELECT id FROM a_Person) AND idp NOT IN (SELECT id FROM a_Atom);' auUniverse -U aumaster
