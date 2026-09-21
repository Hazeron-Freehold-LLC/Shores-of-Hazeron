-----------------------
-- Universe Database --
-----------------------

-- Database name: auUniverse
-- Owner: aumaster
-- Password: Light
-- User: audog
-- Password: Gold

-- Table Naming Convention
--
-- a_ Atoms. Corresponding Da classes exist.
-- b_ Base Economic Systems. Corresponding Db classes exist.
-- e_ Extra atom data. Corresponding De classes exist.
-- g_ Global/Game. Corresponding Dg classes exist.
-- n_ Spacecraft designs. Corresponding Dn classes exist.
-- z_ Zoology. Corresponding Dz classes exist.


---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
----- Global Game Data -----

-- Database Object Ids
-- This table will only contain one record.
-- It is used by the Scene Master to remember the highest ids doled out to Scene
-- Managers.
--DROP TABLE g_NextIds;
CREATE TABLE g_NextIds
(
  atom                        bigint default 1,
  buildingDesign              bigint default 1,
  empire                      bigint default 1,
  itemDesign                  bigint default 1,
  spaceShipDesign             bigint default 1,
  story                       bigint default 1
);
GRANT ALL ON g_NextIds TO audog;
SELECT * from g_NextIds;


-- Established empires information.
--DROP TABLE g_Empire;
CREATE TABLE g_Empire
(
  id               bigint primary key unique,
  name             text,                 -- Name of empire
  immig            smallint default 1,   -- 0=open, 1=restricted
  immigPassCrypt   bytea,                -- SHA1 encrypted password when immig=1
  rightToFound     smallint default 3,   -- DgEmpire::RIGHT_TO_FOUND enum value default FoundOnNeutral
  flag             bytea,                -- Flag image blob. This is a QImage
  capitalCity      bigint DEFAULT 0,     -- id of city designated as the capitol
  capitalDNA       bytea,                -- DNA at the capitol city. Deprecated; replaced by capitalSpec
  capitalSpec      bytea,                -- Species at the capitol city.
  idg              integer default 3,    -- g_Government.id default is Dictatorship
  rewCapBase       integer default 0,
  rewCapCapital    integer default 0,
  rewCapCapitalGalaxy integer default 0,
  rewCapCapitalSector integer default 0,
  rewCapCity       integer default 0,
  rewCapSpaceport  integer default 0,
  rewCapSpaceship  integer default 0,
  rewCapStarship   integer default 0,
  rewDmgSpaceport  integer default 0,
  rewDmgSpaceship  integer default 0,
  rewDmgStarship   integer default 0,
  rewDiscHabitable integer default 0,
  rewDiscWorm      integer default 0,
  rewKillEnemy     integer default 0,
  rewSalary        integer default 0,
  rewSpotCity      integer default 0,
  rewSpotEnemy     integer default 0,
  taxIncome        smallint default 0,
  taxSales         smallint default 0,
  taxSubsidy       smallint default 0,
  defaultStance    smallint default 1,   -- default is 'neutral'
  stance           bytea,
  commerce         bytea,
  citizens         bytea,
  cities           bytea,
  capturemsg       text,                 -- Repurposed for use as welcome message on join empires page.
  trespass         smallint default 1,
  contrailHue      smallint default 170, -- Hue of contrails 0-255
  contrailSat      smallint default 127, -- Saturation of contrails 64-191
  cityDebt         smallint default 1,   -- 0=no, 1=yes
  zoneBuildPolicy  smallint default 0,   -- Bitmask: b0=Friend, b1=Neutral, b2=Enemy, b3=Vassal
  announcements    text,                 -- announcements to all members
  role             smallint default 0,   -- 0=normal empire, 1=pirate, 2=admin, 3=arena team, 4=arena observer
  theme            bytea,                -- theme colors of ships, etc.
  bdprefs          bytea,
  online           timestamp,            -- Last time an avatar of this empire reported being on line.
  scprefs          bytea
);
GRANT ALL ON g_Empire TO audog;
CREATE INDEX g_Empire_id_index ON g_Empire USING btree (id);
-- Pirate Empires. Flags can be set after restart by an agent.
INSERT INTO g_Empire (id,role,defaultStance,immig,name) VALUES ( 1, 1, 2, 2, 'Akson');
INSERT INTO g_Empire (id,role,defaultStance,immig,name) VALUES ( 2, 1, 2, 2, 'Balorite');
INSERT INTO g_Empire (id,role,defaultStance,immig,name) VALUES ( 3, 1, 2, 2, 'Dendrae');
INSERT INTO g_Empire (id,role,defaultStance,immig,name) VALUES ( 4, 1, 2, 2, 'Haxu');
INSERT INTO g_Empire (id,role,defaultStance,immig,name) VALUES ( 5, 1, 2, 2, 'Kla''tra');
INSERT INTO g_Empire (id,role,defaultStance,immig,name) VALUES ( 6, 1, 2, 2, 'Malacon');
INSERT INTO g_Empire (id,role,defaultStance,immig,name) VALUES ( 7, 1, 2, 2, 'Myntaka');
INSERT INTO g_Empire (id,role,defaultStance,immig,name) VALUES ( 8, 1, 2, 2, 'Ogar');
INSERT INTO g_Empire (id,role,defaultStance,immig,name) VALUES ( 9, 1, 2, 2, 'Otari');
INSERT INTO g_Empire (id,role,defaultStance,immig,name) VALUES (10, 1, 2, 2, 'Seledon');
INSERT INTO g_Empire (id,role,defaultStance,immig,name) VALUES (11, 1, 2, 2, 'Syth');
INSERT INTO g_Empire (id,role,defaultStance,immig,name) VALUES (12, 1, 2, 2, 'Tassad');
INSERT INTO g_Empire (id,role,defaultStance,immig,name) VALUES (13, 1, 2, 2, 'Vilmorti');
INSERT INTO g_Empire (id,role,defaultStance,immig,name) VALUES (14, 1, 2, 2, 'Vreen');
INSERT INTO g_Empire (id,role,defaultStance,immig,name) VALUES (15, 1, 2, 2, 'Zuul');
INSERT INTO g_Empire (id,role,defaultStance,immig,name) VALUES (20, 4, 0, 2, 'Observer');
INSERT INTO g_Empire (id,role,defaultStance,immig,name) VALUES (21, 3, 2, 2, 'Alpha Fleet');
INSERT INTO g_Empire (id,role,defaultStance,immig,name) VALUES (22, 3, 2, 2, 'Zulu Fleet');
SELECT * FROM g_Empire;


-- Stories that are open
--DROP TABLE g_Story;
CREATE TABLE g_Story
(
  id                          bigint primary key unique,    -- story id
  block                       text,                         -- current block label
  blockStart                  timestamp,                    -- Time the block was started
  people                      bytea,                        -- ids of people involved in story
  vars                        bytea,                        -- variables and their values
  script                      integer,                      -- script id
  canxOk                      boolean                       -- false when story must not be dropped
);
GRANT ALL ON g_Story TO audog;
CREATE INDEX g_Story_id_index ON g_Story USING btree (id);
SELECT * FROM g_Story;

---+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
----- Spacecraft Designs -----

-- A spacecraft design is used to define the deck plans of a spacecraft.
-- The design is sufficient to enable complete 3D skinning of the ship, inside and out.
-- Design may only be modified while it is in progress. A finished design may be
-- constructed in the universe but cannot be loaded in the design environment by anyone
-- but the architect. A published design may be constructed in the universe and can be
-- loaded by others.
--DROP TABLE n_ScDesign;
CREATE TABLE n_ScDesign
(
  id                          bigint primary key unique,
  name                        text,                         -- Name of this design.
  ida                         bigint,                       -- id of architect a_Person
  archname                    text,                         -- Name of the architect a_Person
  state                       smallint,                     -- 0=writable, 1=read only, 2=read only and public, 3=view only,
                                                            -- 4=read only and published, 5=deleted
  textures                    bytea,                        -- custom texture and texture palette
  -- Design Properties
  modules                     bytea,                        -- system modules designed
  parts                       bytea,                        -- parts that make up the design
  landingOrient               smallint,                     -- preferred landing orientation LANDING_ORIENT enum
  stationOrient               smallint,                     -- preferred orbit orientation for stations STATION_ORIENT enum
  streamline                  smallint,                     -- hull streamlining 0=none, 1=atmospheric, 2=submersible
  structure                   smallint,                     -- DnSpacecraft::STRUCTURE
  techLevelMin                smallint,                     -- minimum quality at which design must be manufactured
  -- Hull volume distribution.
  armor                       integer,                      -- hull armor volume in cubic meters
  bays                        bytea,                        -- station with module, part ids and associated hull volumes
  capacitor                   integer,                      -- capacitor volume in cubic meters
  cryoBerths                  integer,                      -- cryo berth volume in cubic meters. One berth per cubic meter.
  ftlDrives                   bytea,                        -- station with module, part ids and associated hull volumes
  fuel                        integer,                      -- fuel volume in cubic meters.
  hold                        integer,                      -- hold volume in cubic meters.
  lifeSupport                 integer,                      -- life support volume in cubic meters.
  maneuverDrive               integer,                      -- maneuver drive volume in cubic meters.
  powerPlant                  integer,                      -- power plant volume in cubic meters.
  sensors                     integer,                      -- sensors volume in cubic meters.
  shields                     integer,                      -- shields volume in cubic meters.
  skeleton                    integer,                      -- skeleton volume in cubic meters.
  -- Design Analysis Results
  hullVolume                  integer,                      -- overall volume of the hull in cubic meters
  techLevel                   smallint                      -- Quality required to manufacture the design.
);
GRANT ALL ON n_ScDesign TO audog;
CREATE INDEX n_ScDesign_id_index ON n_ScDesign USING btree (id);
CREATE INDEX n_ScDesign_architect_index ON n_ScDesign USING btree (state, ida);
SELECT * FROM n_ScDesign;

---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
-- Spacecraft blueprint exchange record.

--DROP TABLE n_ScExchange;
CREATE TABLE n_ScExchange
(
  id                          bigint primary key unique,    -- Id of the design.
  state                       smallint,                     -- AuDesign::STATE
  canCopy                     boolean,                      -- True if buyer can copy to clipboard or save to file. Implies canView.
  canView                     boolean,                      -- True if buyer can view in design studio.
  empire                      bigint,                       -- Empire of publisher.
  minStance                   smallint,                     -- Minimum POLITICAL_STANCE toward the buyer.
  price                       integer,                      -- Price in cronos to buy the design.
  useOnce                     boolean,                      -- True if buyer can buy one or set one manufacturing process.
  whenPub                     timestamp,                    -- When this record was published.
  report                      bytea                         -- AuScReport blob
);
GRANT ALL ON n_ScExchange TO audog;
CREATE INDEX n_ScExchange_id_index ON n_ScExchange USING btree (id);
SELECT * FROM n_ScExchange;

---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
-- Spacecraft blueprint ratings.

--DROP TABLE n_ScRating;
CREATE TABLE n_ScRating
(
  comment                     text,                         -- Rater comment.
  designid                    bigint,                       -- Id of the design.
  rating                      smallint,                     -- Rating given to the design: 1 to 5.
  rid                         bigint,                       -- Avatar atom id who rated a design. Each only once per design.
  rname                       text,                         -- Avatar atom name who rated a design.
  whenRated                   timestamp                     -- When this design was last rated by rater.
);
GRANT ALL ON n_ScRating TO audog;
CREATE INDEX n_ScRating_id_index ON n_ScRating USING btree (designid);
CREATE INDEX n_ScRating_multi_index ON n_ScRating USING btree (designid, rid);
SELECT * FROM n_ScRating;

---+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
----- Building Designs -----

--DROP TABLE n_BdDesign;
CREATE TABLE n_BdDesign
(
  id                          bigint primary key unique,
  name                        text,                         -- Name of this design.
  ida                         bigint,                       -- id of architect a_Person
  archname                    text,                         -- Name of the architect a_Person
  state                       smallint,                     -- 0=writable, 1=read only, 2=read only and public, 3=view only,
                                                            -- 4=read only and published, 5=deleted
  textures                    bytea,                        -- custom texture and texture palette
  -- Design Properties
  modules                     bytea,                        -- system modules designed
  parts                       bytea,                        -- parts that make up the design
  structure                   smallint,                     -- DnSpacecraft::STRUCTURE
  iid                         smallint,                     -- Intended industry. Not a restriction.
  material                    integer,                      -- BUILDMATERIAL
  -- Hull volume distribution.
  arenas                      integer,                      -- volume in cubic meters
  armor                       integer,                      -- volume in cubic meters
  auditoriums                 integer,                      -- volume in cubic meters
  bays                        bytea,                        -- station with module, part ids and associated hull volumes
  capacitor                   integer,                      -- volume in cubic meters
  fuel                        integer,                      -- volume in cubic meters.
  hold                        integer,                      -- volume in cubic meters.
  homesLarge                  integer,                      -- volume in cubic meters.
  homesMedium                 integer,                      -- volume in cubic meters.
  homesSmall                  integer,                      -- volume in cubic meters
  lifeSupport                 integer,                      -- volume in cubic meters.
  lounges                     integer,                      -- volume in cubic meters.
  offices                     integer,                      -- volume in cubic meters.
  powerPlant                  integer,                      -- volume in cubic meters.
  sensors                     integer,                      -- volume in cubic meters.
  shields                     integer,                      -- volume in cubic meters.
  shops                       integer,                      -- volume in cubic meters.
  skeleton                    integer,                      -- skeleton volume in cubic meters.
  stores                      integer,                      -- volume in cubic meters.
  -- Design Analysis Results
  hullVolume                  integer                       -- overall volume of the hull in cubic meters
);
GRANT ALL ON n_BdDesign TO audog;
CREATE INDEX n_BdDesign_id_index ON n_BdDesign USING btree (id);
CREATE INDEX n_BdDesign_architect_index ON n_BdDesign USING btree (state, ida);
SELECT * FROM n_BdDesign;

---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
-- Building blueprint exchange record.

--DROP TABLE n_BdExchange;
CREATE TABLE n_BdExchange
(
  id                          bigint primary key unique,    -- Id of the design.
  state                       smallint,                     -- AuDesign::STATE
  canCopy                     boolean,                      -- True if buyer can copy to clipboard or save to file. Implies canView.
  canView                     boolean,                      -- True if buyer can view in design studio.
  empire                      bigint,                       -- Empire of publisher.
  minStance                   smallint,                     -- Minimum POLITICAL_STANCE toward the buyer.
  price                       integer,                      -- Price in cronos to buy the design.
  useOnce                     boolean,                      -- True if buyer can buy one or set one manufacturing process.
  whenPub                     timestamp,                    -- When this record was published.
  report                      bytea                         -- AuBdReport blob
);
GRANT ALL ON n_BdExchange TO audog;
CREATE INDEX n_BdExchange_id_index ON n_BdExchange USING btree (id);
SELECT * FROM n_BdExchange;

---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
-- Building blueprint ratings.

--DROP TABLE n_BdRating;
CREATE TABLE n_BdRating
(
  comment                     text,                         -- Rater comment.
  designid                    bigint,                       -- Id of the design.
  rating                      smallint,                     -- Rating given to the design: 1 to 5.
  rid                         bigint,                       -- Avatar atom id who rated a design. Each only once per design.
  rname                       text,                         -- Avatar atom name who rated a design.
  whenRated                   timestamp                     -- When this design was last rated by rater.
);
GRANT ALL ON n_BdRating TO audog;
CREATE INDEX n_BdRating_id_index ON n_BdRating USING btree (designid);
CREATE INDEX n_BdRating_multi_index ON n_BdRating USING btree (designid, rid);
SELECT * FROM n_BdRating;

---+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
----- Item Designs -----

--DROP TABLE n_ItDesign;
CREATE TABLE n_ItDesign
(
  id                          bigint primary key unique,
  name                        text,                         -- Name of this design.
  ida                         bigint,                       -- id of architect a_Person
  archname                    text,                         -- Name of the architect a_Person
  state                       smallint,                     -- 0=writable, 1=read only, 2=read only and public, 3=view only,
                                                            -- 4=read only and published, 5=deleted
  textures                    bytea,                        -- custom texture and texture palette
  -- Design Properties
  cid                         smallint,                     -- COMMODITYID
  groundOrient                smallint,                     -- GROUND_ORIENT
  parts                       bytea                         -- parts that make up the design
);
GRANT ALL ON n_ItDesign TO audog;
CREATE INDEX n_ItDesign_id_index ON n_ItDesign USING btree (id);
CREATE INDEX n_ItDesign_architect_index ON n_ItDesign USING btree (state, ida);
SELECT * FROM n_ItDesign;

---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
----- Zoology -----

--DROP TABLE g_Species;
CREATE TABLE g_Species
(
 species                     bytea primary key unique,     -- AuSpecies
 name                        text,                         -- name assigned to the specie
 whoNamed                    bigint,                       -- atom id of the person who named this specie
 whenNamed                   timestamp                     -- when this specie was named
);
GRANT ALL ON g_Species TO audog;
CREATE INDEX g_Species_species_index ON g_Species USING btree (species);
CREATE INDEX g_Species_whoNamed_index ON g_Species USING btree (whoNamed);
SELECT * FROM g_Species;

---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
----- Atoms -----

-- Every object in the universe, including the universe, is an atom.
-- Atoms are organized into a hierarchy.
-- Class: DaAtom
--DROP TABLE a_Atom CASCADE;
CREATE TABLE a_Atom
(
  id                          bigint primary key unique,
  idp                         bigint,                       -- id of parent atom in hierarchy. == id if at top.
  locX                        float8,                       -- Location and Rotation are used to construct the Atom's transformation matrix. Atoms are never scaled.
  locY                        float8,
  locZ                        float8,
  rotX                        float8,
  rotY                        float8,
  rotZ                        float8,
  timeCreate                  timestamp,                    -- Time this atom was created.
  timeModified                timestamp,                    -- Time this atom was last modified.
  timeTick                    timestamp,                    -- Time as precisely as possible (down to the millisecond or better) when last ticked.
  timeTock                    timestamp,                    -- Time as precisely as possible (down to the millisecond or better) when last tocked.
  timeDeath                   timestamp                     -- Time this atom died or will die. Removed from database when dead.
);
GRANT ALL ON a_Atom TO audog;
CREATE INDEX a_Atom_id_index ON a_Atom USING btree (id);
SELECT * FROM a_Atom;

----- Environment -----

-- A universe.
--DROP TABLE a_Universe;
CREATE TABLE a_Universe
(
) INHERITS (a_Atom);
GRANT ALL ON a_Universe TO audog;
CREATE INDEX a_Universe_id_index ON a_Universe USING btree (id);
CREATE INDEX a_Universe_idp_index ON a_Universe USING btree (idp);
SELECT * FROM a_Universe;

-- Galaxies populate universes.
--DROP TABLE a_Galaxy;
CREATE TABLE a_Galaxy
(
 galaxy                                 smallint                -- GALAXY enum from dbdef.h, 15=ShoresOfHazeron, 17=VeilOfTargoss
) INHERITS (a_Atom);
GRANT ALL ON a_Galaxy TO audog;
CREATE INDEX a_Galaxy_id_index ON a_Galaxy USING btree (id);
CREATE INDEX a_Galaxy_idp_index ON a_Galaxy USING btree (idp);
SELECT * FROM a_Galaxy;

-- Sectors subdivide galaxies.
--DROP TABLE a_Sector;
CREATE TABLE a_Sector
(
  name                        text                          -- The name of the sector.
) INHERITS (a_Atom);
GRANT ALL ON a_Sector TO audog;
CREATE INDEX a_Sector_id_index ON a_Sector USING btree (id);
CREATE INDEX a_Sector_idp_index ON a_Sector USING btree (idp);
CREATE INDEX a_Sector_multi_index ON a_Sector USING btree (idp, locX, locY, locZ);
SELECT * FROM a_Sector;

-- Solar systems populate sectors.
--DROP TABLE a_SolarSystem;
CREATE TABLE a_SolarSystem
(
 name                                  text,                    -- The name of the solar system.
 exploredBy                            bytea,
 scannedBy                             bytea,
 detectedBy                            bytea,
 hengeDest                             bytea,                   -- Cache of henge info on unfinalized systems.
 wormholes                             bytea,
 genHab                                smallint,                -- HABITIBILITY value used when CreatePlanets was called
                                                                -- Will be 1=System or 2=Homeworld. All other values taken as System
 genSeed                               bigint default 0,        -- Random seed value used to generate the system contents.
 lastVisit                             timestamp,               -- Last time a player was on-line in this solar system.
 lastActiveLand                        timestamp,               -- Last time pinged by an avatar in an active account.
 lastOfficerVisit                      timestamp,               -- Last time an AI officer in command of a ship was in this system
 lastPirateStation                     timestamp,               -- Reassigned to m_LastFlagInfluence. Pirate stations were nerfed.
 powerConsumed                         integer,                 -- power consumed in the system
 warpSignature                         integer,                 -- power consumed by warp drives in the system
 final                                 boolean                  -- True while contents are finalized
) INHERITS (a_Atom);
GRANT ALL ON a_SolarSystem TO audog;
CREATE INDEX a_SolarSystem_id_index ON a_SolarSystem USING btree (id);
CREATE INDEX a_SolarSystem_idp_index ON a_SolarSystem USING btree (idp);
SELECT * FROM a_SolarSystem;

-- Stars populate solar systems.
-- Their parents are always solar systems.
--DROP TABLE a_Star;
CREATE TABLE a_Star
(
 specType          smallint,           -- spectral type
 specDec           smallint,           -- spectral decimal classification
 specSize          smallint,           -- spectral size
 radius            smallint,           -- radius in orbits
 orbit             smallint,           -- orbit number
 habZone           smallint,           -- orbit of habitable zone
 tempK             int,                -- Surface temperature in kelvin (1,000 - 40,000)
 orbitZones        character(16)       -- Character encoding of orbit zone status.
                                       -- Orbit zone is character index (0-15).
                                       -- Character at each orbit zone index will be one of:
                                       --   '0' = Inside Star
                                       --   '1' = Unavailable
                                       --   '2' = Inferno
                                       --   '3' = Inner
                                       --   '4' = Habitable
                                       --   '5' = Outer
                                       --   '6' = Frigid
) INHERITS (a_Atom);
GRANT ALL ON a_Star TO audog;
CREATE INDEX a_Star_id_index ON a_Star USING btree (id);
CREATE INDEX a_Star_idp_index ON a_Star USING btree (idp);
SELECT * FROM a_Star;

--DROP TABLE a_World;
CREATE TABLE a_World
(
 ids               bigint,              -- atom id of parent solar system
 name              text,                -- Name of the planet.
 orbitRadius       float4,              -- Orbit radius in Au.
 orbitZone         smallint             -- OZONE_ enum value
) INHERITS (a_Atom);
GRANT ALL ON a_World TO audog;
CREATE INDEX a_World_id_index ON a_World USING btree (id);
CREATE INDEX a_World_idp_index ON a_World USING btree (idp);
SELECT * FROM a_World;

--DROP TABLE a_WorldGasGiant;
CREATE TABLE a_WorldGasGiant
(
 atmDensity        smallint,            -- 0-100 percent density
 atmType           smallint,            -- ATMTYPE_ enum value
 claimedBy         bigint,              -- empire id who claims the world
 claimLon          float4,              -- longitude where claim was made
 claimLat          float4,              -- latitude where claim was made
 claimId           bigint,              -- person id who staked the claim
 claimName         text,                -- person name who staked the claim
 claimTime         timestamp,           -- when last claimed
 dna               bytea,               -- Indigeneous population dna.
 indSpec           bytea,               -- Indigenous species
 fauna             bytea,               -- Fauna data when habitable
 flora             bytea,               -- Flora data when foliage present.
 fogOfWar          bytea,               -- FogOfWar data
 geoFeatures       bytea,               -- Geological features, e.g. craters, chasms, volcanoes
 radius            smallint,            -- Radius in 1000s of km. range is 20-40
 terrain           bytea                -- Terrain data
) INHERITS (a_World);
GRANT ALL ON a_WorldGasGiant TO audog;
CREATE INDEX a_WorldGasGiant_id_index ON a_WorldGasGiant USING btree (id);
CREATE INDEX a_WorldGasGiant_idp_index ON a_WorldGasGiant USING btree (idp);
SELECT * FROM a_WorldGasGiant;

-- This is a rocky ring like those found around Saturn, not a ringworld.
--DROP TABLE a_WorldRing;
CREATE TABLE a_WorldRing
(
) INHERITS (a_World);
GRANT ALL ON a_WorldRing TO audog;
CREATE INDEX a_WorldRing_id_index ON a_WorldRing USING btree (id);
CREATE INDEX a_WorldRing_idp_index ON a_WorldRing USING btree (idp);
SELECT * FROM a_WorldRing;

--DROP TABLE a_WorldLL;
CREATE TABLE a_WorldLL
(
 captId            bigint default 0,    -- Id of captain from this planet. Only in habitable zone.
 captName          text,                -- Name of captain from this planet. Only in habitable zone.
 claimedBy         bigint,              -- empire id who claims the world
 claimLon          float4,              -- longitude where claim was made
 claimLat          float4,              -- latitude where claim was made
 claimId           bigint,              -- person id who staked the claim
 claimName         text,                -- person name who staked the claim
 claimTime         timestamp,           -- when last claimed
 geoFeatures       bytea,               -- Geological features, e.g. craters, chasms, volcanoes
 village           bytea                -- Village(s).
) INHERITS (a_World);
GRANT ALL ON a_WorldLL TO audog;
-- CREATE INDEX a_WorldLL_id_index ON a_WorldLL USING btree (id);
-- CREATE INDEX a_WorldLL_idp_index ON a_WorldLL USING btree (idp);
SELECT * FROM a_WorldLL;

--DROP TABLE a_WorldGlobe;
CREATE TABLE a_WorldGlobe
(
 atmDensity        smallint,            -- 0-100 percent density
 atmType           smallint,            -- ATMTYPE_ enum value
 dna               bytea,               -- Indigeneous population dna.
 indSpec           bytea,               -- Indigenous species
 fauna             bytea,               -- Fauna data when habitable
 flora             bytea,               -- Flora data when foliage present.
 fogOfWar          bytea,               -- FogOfWar data
 radius            smallint,            -- Radius in 100s of km. 0=ring. 10-29=moon. 30-120=planet, 200-400=gas giant, 500=ringworld
 terrain           bytea,               -- Terrain data
 water             smallint             -- 0-100 percent surface coverage
) INHERITS (a_WorldLL);
GRANT ALL ON a_WorldGlobe TO audog;
CREATE INDEX a_WorldGlobe_id_index ON a_WorldGlobe USING btree (id);
CREATE INDEX a_WorldGlobe_idp_index ON a_WorldGlobe USING btree (idp);
SELECT * FROM a_WorldGlobe;

-- This is one segment of a ringworld.
--DROP TABLE a_WorldRingSection;
CREATE TABLE a_WorldRingSection
(
 atmDensity        smallint,            -- 0-100 percent density
 atmType           smallint,            -- ATMTYPE_ enum value
 dna               bytea,               -- Indigeneous population dna.
 indSpec           bytea,               -- Indigenous species
 fauna             bytea,               -- Fauna data when habitable
 flora             bytea,               -- Flora data when foliage present.
 sectionIndex      smallint,            -- Index of WORLD_RingworldSection
 terrain           bytea,               -- Terrain data
 water             smallint             -- 0-100 percent surface coverage
) INHERITS (a_WorldLL);
GRANT ALL ON a_WorldRingSection TO audog;
CREATE INDEX a_WorldRingSection_id_index ON a_WorldRingSection USING btree (id);
CREATE INDEX a_WorldRingSection_idp_index ON a_WorldRingSection USING btree (idp);
SELECT * FROM a_WorldRingSection;

--DROP TABLE a_Teleporter;
CREATE TABLE a_Teleporter
(
  node                        bigint,            -- id of dest teleporter on org teleporter; 0 on dest teleporter
  teleporters                 bytea,             -- Byte array of AuPoints at each teleporter location
  spotType                    smallint,
  maxRange                    float8             -- maximum range of teleporter based on quality of originator
) INHERITS (a_Atom);
GRANT ALL ON a_Teleporter TO audog;
CREATE INDEX a_Teleporter_id_index ON a_Teleporter USING btree (id);
CREATE INDEX a_Teleporter_idp_index ON a_Teleporter USING btree (idp);
SELECT * FROM a_Teleporter;


----- Units -----

-- A unit is the base class for all Atoms that can be "owned" by an empire.
--DROP TABLE a_Unit CASCADE;
CREATE TABLE a_Unit
(
 name                                   text,                   -- Name of this unit. E.g. Bob, Enterprise, Whiskers
 allegiance                             bigint,                 -- id of g_Empire who owns this Unit.
 arenaTeam                              smallint,               -- Arena team Id; 0=observer, else = empire id
 conditions                             bytea,                  -- AuConditionList in a blob
 damagehistory                          bytea                   -- AuUnitDamage list
) INHERITS (a_Atom);
GRANT ALL ON a_Unit TO audog;
SELECT * FROM a_Unit;

-- A vessel is the base class of all spacecraft, regardless of design style.
--DROP TABLE a_Vessel CASCADE;
CREATE TABLE a_Vessel
(
 state               smallint,        -- DaAtom::m_State. Spacecraft can be dormant. All other atoms can only be Normal.
 accessPolicy        smallint,        -- ACCESS_POLICY
 pilotUpPolicy       smallint,        -- PILOT_UP_POLICY
 shieldPolicy        smallint,        -- SHIELD_POLICY
 shieldFuelPolicy    smallint,        -- SHIELDFUEL_POLICY
 atRest              boolean,         -- true while at rest
 vecX                float8,          -- Lateral speed in fps
 vecY                float8,          -- Forward speed in fps
 vecZ                float8,          -- Vertical speed in fps
 throttle            smallint,        -- throttle setting
 throttleLateral     smallint,        -- momentary lateral throttle setting
 throttleLong        smallint,        -- momentary longitudinal throttle setting
 throttleVertical    smallint,        -- momentary vertical throttle setting
 ownerId             bigint,          -- Id of atom who owns this ship, for privately owned ships.
 ownerName           text,            -- Name of atom who owns this ship, for privately owned ships.
 durEngine           integer,         -- SkewDur values.
 skewEngine          integer,         -- Skew values.
 fuel                float4,          -- Fuel quantity.
 fuelqual            smallint,        -- Fuel quality 1-255
 rotSpeeds           integer,         -- RotSpeed X,Y,Z values.
 actBits             integer,         -- ACT_ action bits.  
 capacitor           float4,          -- Capacitor level
 route               bytea,           -- Route of waypoints configured at navigator station
 cargo               bytea,
 damage              bytea,           -- AuSpacecraftDamage
 roster              bytea,           -- AuRoster
 bank                float8 default 0,-- Bank account balance
 bankSubsidyTax      float8 default 0,-- Subsidy taxes collected for the empire.
 systems             bytea,           -- auShipSystemArray
 shieldFront         float4,          -- 0..1 ratio of available shields allocated. shield allocation fields all add up to 1
 shieldBack          float4,          -- 0..1 ratio of available shields allocated. shield allocation fields all add up to 1
 shieldLeft          float4,          -- 0..1 ratio of available shields allocated. shield allocation fields all add up to 1
 shieldRight         float4,          -- 0..1 ratio of available shields allocated. shield allocation fields all add up to 1
 shieldTop           float4,          -- 0..1 ratio of available shields allocated. shield allocation fields all add up to 1
 shieldBottom        float4,          -- 0..1 ratio of available shields allocated. shield allocation fields all add up to 1
 powerPlantShunts    smallint,        -- flags for which power plant shunts are active on the ship
 techLevel           smallint,        -- quality at which the ship was manufactured. may be higher than the design quality
 whereMade           text,            -- Brief description of where made. Typically "Empire city City on World in System".
 mission             bytea,           -- Current mission assigned.
 games               bytea,           -- Games in progress at game tables.
 repairSecs          integer,         -- seconds remaining to finish repairs
 baySetting          bytea,           -- weapon bay settings. which ids with power for weapons and commodity id for harvesters
 ratdna              bytea,           -- DNA of rat for this ship. Obsolete. Replaced by ratspecies.
 ratspec             bytea,           -- Species of rat for this ship
 monster             smallint         -- monster encounter counter
) INHERITS (a_Unit);
GRANT ALL ON a_Vessel TO audog;
SELECT * FROM a_Vessel;

-- New style spacecraft designed with n_ScDesign records.
--DROP TABLE a_Sc;
CREATE TABLE a_Sc
(
 idDesign            bigint,          -- id of n_ScDesign
 switches            integer,
 designRpt           bytea,
 airPressures        bytea,
 doorStates          bytea,
 ftlDrives           bytea,
 landingGear         bytea,
 landingSystems      bytea,
 launchers           bytea,            -- Space vehicle launchers
 lifts               bytea,
 roomLights          bytea,
 transporters        bytea,
 turretPos           bytea
) INHERITS (a_Vessel); -- NOTE: In current live db, inherits a_Unit and all a_Vessel fields are here.
GRANT ALL ON a_Sc TO audog;
CREATE INDEX a_Sc_id_index ON a_Sc USING btree (id);
CREATE INDEX a_Sc_idp_index ON a_Sc USING btree (idp);
CREATE INDEX a_Sc_allegiancestate_index ON a_Sc USING btree (allegiance, state);
CREATE INDEX a_Sc_idpstate_index ON a_Sc USING btree (idp, state);
SELECT * FROM a_Sc;

-- A vehicle is a simple unit.
--DROP TABLE a_Vehicle;
CREATE TABLE a_Vehicle
(
 cid                 integer,         -- id of vehicle commodity
 qual                smallint,        -- quality at which the vehicle was manufactured
 actBits             integer,         -- ACT_ action bits.
 atRest              boolean,         -- true while at rest
 vecX                float8,          -- Lateral speed in fps
 vecY                float8,          -- Forward speed in fps
 vecZ                float8,          -- Vertical speed in fps
 throttle            smallint,        -- throttle setting
 throttleLateral     smallint,        -- lateral throttle setting
 throttleLong        smallint,        -- longitudinal throttle setting
 throttleVertical    smallint,        -- vertical throttle setting
 switches            smallint,        -- switches
 hp                  smallint,        -- Hit points remaining
 techLevel           smallint,        -- quality at which the vehicle was manufactured
 fuel                smallint,        -- fuel (1 unit per minute)
 ord                 bytea,           -- Ordnance loaded.
 motherShip          bigint,          -- Assigned spacecraft id, if any.
 motherShipName      text             -- Assigned spacecraft name, if any.
) INHERITS (a_Unit);
GRANT ALL ON a_Vehicle TO audog;
CREATE INDEX a_Vehicle_id_index ON a_Vehicle USING btree (id);
CREATE INDEX a_Vehicle_idp_index ON a_Vehicle USING btree (idp);
SELECT * FROM a_Vehicle;

-- A creature in the environment.
--DROP TABLE a_Creature;
CREATE TABLE a_Creature
(
 sex                 smallint,        -- 1=male, 0=female. This is now the chromatid in reverse value order: 1=Alpha, 0=Beta
 islefty             boolean,         -- true if left-handed
 dna                 bytea,           -- DNA for obsolete DpBody
 dna4                bytea,           -- Dynamic DNA could be DhDNA or AuDNA
 bodyQ               smallint,        -- Current body Q
 hp                  smallint,        -- Hit points remaining
 atRest              boolean,         -- true while at rest
 vecX                float8,          -- Lateral speed in fps
 vecY                float8,          -- Forward speed in fps
 vecZ                float8,          -- Vertical speed in fps
 stamina             smallint,        -- Stamina points remaining
 hunger              smallint,        -- Hunger satiated
 pose                smallint,        -- General body pose.
 seatIndex           smallint,
 whichConsole        integer,         -- Console subsystem identifier when pose is a console.
 stationId           integer,         -- Console station id: part id for AuDesigns, detail id for DnSpaceShips
 minsToFullGrown     smallint,        -- number of minutes remaining for child creature to be full grown
 hair                smallint,        -- haircut
 shave               smallint,        -- shave
 skills              bytea            -- AuSkillSet
) INHERITS (a_Unit);
GRANT ALL ON a_Creature TO audog;
SELECT * FROM a_Creature;

-- A creature in the environment.
--DROP TABLE a_Animal;
CREATE TABLE a_Animal
(
) INHERITS (a_Creature);
GRANT ALL ON a_Animal TO audog;
CREATE INDEX a_Animal_id_index ON a_Animal USING btree (id);
CREATE INDEX a_Animal_idp_index ON a_Animal USING btree (idp);
SELECT * FROM a_Animal;

-- A non-player character
--DROP TABLE a_Citizen;
CREATE TABLE a_Citizen
(
 inv               bytea,              -- DeInventoryItemArray
 ship              bigint,             -- id of ship this citizen is assigned
 berthType         smallint,           -- berth type aboard ship
 lineage           smallint,           -- 0=none, 1=citizen, 2=demiavatar, 3=avatar
 orders            bytea,              -- Order vector Z
 posture           smallint            -- CITIZEN_POSTURE enum value, from dbdef.h
) INHERITS (a_Creature);
GRANT ALL ON a_Citizen TO audog;
SELECT * FROM a_Citizen;

-- A generic non-player character to replace/combine the separate ones below.
--DROP TABLE a_Npc;
CREATE TABLE a_Npc
(
 bank              float8 default 0,   -- Bank account balance.
 companion         bigint default 0,   -- Companion avatar id, if any.
 portBd            bigint default 0,   -- Bd id at which the last spacecraft was boarded|debarked. May be 0.
 spouseName        text,               -- Blank is unmarried
 indestructible    boolean,            -- True if I am indestructible.
 interlocutor      bigint default 0,   -- Person with whom I am conversing.
 role              smallint default 0, -- NPC_ROLE
 con               boolean,            -- Officer: true if this officer thinks he has the con
 log               bytea,              -- Officer: Captain's Log
 logtime           timestamp,          -- Officer: when log last sent to chain of command
 sectorId          bigint default 0,   -- Officer: Sector id from which this officer originated
 sectorName        text,               -- Officer: Sector name from which this officer originated
 systemId          bigint default 0,   -- Officer: Solar System id from which this officer originated
 systemName        text,               -- Officer: Solar System name from which this officer originated
 worldId           bigint default 0,   -- Officer: Planet id from which this officer originated
 worldName         text,               -- Officer: Planet name from which this officer originated
 worldBodyType     smallint,           -- Officer: BODY_TYPE of home world
 galaxy            smallint,           -- Officer: Galaxy of origin. GALAXY from dbdef.h, 15=ShoresOfHazeron, 17=VeilOfTargoss
 homesick          smallint,           -- Officer: Days home city has not had a city owned by my empire.
 homesickTime      timestamp,          -- Officer: When homesick value was last changed.
 garrisonBd        bigint default 0,   -- Troop: Bd id to which this troop is assigned. May be 0.
 story             bigint,             -- Character: story instance id
 wantCID           smallint,           -- Character: Commodity desired by this character
 wantQ             smallint,           -- Character: Commodity desired by this character, 0 = don't care
 wantEoO           bigint,             -- Character: Empire of origin desired by this character, 0 = don't care
 wantIns           text                -- Character: Inscription text wanted, blank = don't care
) INHERITS (a_Citizen);
GRANT ALL ON a_Npc TO audog;
CREATE INDEX a_Npc_id_index ON a_Npc USING btree (id);
CREATE INDEX a_Npc_idp_index ON a_Npc USING btree (idp);
SELECT * FROM a_Npc;

-- A person is a player's avatar in the game universe.
--DROP TABLE a_Person;
CREATE TABLE a_Person
(
 bank                                   float8 default 0,       -- Bank account balance.
 loan                                   float8 default 0,       -- Loan account balance. Amount owed to the bank.
 designpass                             text,                   -- Design studio password
 online                                 integer,                -- ACCOUNT type while on line.
 office                                 smallint default 6,     -- Office held in empire. 6=citizen.
 spawnCity                              bigint default 0,       -- City id chosen for spawn.
 spawnCityName                          text,                   -- Name of spawn city.
 spawnSystem                            bigint default 0,       -- Solar system that contains spawn city.
 spawnSystemName                        text,                   -- Name of spawn city's solar system.
 spawnx                                 float default 0,        -- Longitude on globe world.
 spawny                                 float default 0,        -- Latitude on globe world.
 spawnship                              bigint default 0,       -- Spacecraft id chosen for spawn.
 spawnShipIsPrivate                     boolean,                -- True if spawn ship is privately owned.
 spawnShipName                          text,                   -- Name of spawn ship.
 spawnGalaxy                            smallint,               -- GALAXY player was in when they respawned.
 berthType                              smallint,               -- berth type held aboard spawnship
 social                                 integer default 0,      -- Social standing
 usepos                                 smallint default 0,     -- ((AuGearSlot::SLOTTYPE << 8)|which) of current fire/activate item
 usesub                                 smallint default 0,     -- INVENTORYSUB of current fire/activate item
 mappos                                 bytea,                  -- Mappings for hot key selection of current item.
 worms                                  smallint default 0,     -- Wormholes discovered by this person.
 inv                                    bytea,                  -- DeInventoryItemArray
 script                                 bytea,                  -- Story AuScript private to this avatar
 seaChest                               bytea,                  -- DeInventoryItemHash
 jobs                                   bytea,                  -- Open stories and other kinds of jobs.
 memory                                 bytea,                  -- Memories
 buddylist                              bytea,                  -- Buddy list array
 stats                                  bytea,                  -- Statistics blob
 governs                                bytea,                  -- Ids and names of atoms governed by this person.
 agentBits                              smallint,               -- bit mask of special settings relevant to agents
 rewards                                float8 default 0,       -- Rewards owed to player
 arenaEntered                           timestamp,              -- Arena entry time.
 arenaExitLocX                          float8,                 -- loc relative to parent atom when arena entered, for exit
 arenaExitLocY                          float8,                 -- loc relative to parent atom when arena entered, for exit
 arenaExitLocZ                          float8,                 -- loc relative to parent atom when arena entered, for exit
 arenaExitParent                        bigint,                 -- parent atom id when arena entered, for exit
 arenaShipDesign                        bigint,                 -- Arena ship design id, chosen when entering.
 arenaDesignIsSc                        boolean,                -- Arena ship design is from n_ScDesign
 lineage                                smallint,               -- 0=none, 1=citizen, 2=demiavatar, 3=avatar
 dad                                    bigint default 0,       -- Id of father.
 mom                                    bigint default 0,       -- Id of mother.
 orders                                 bytea,                  -- Orders when AI controlled.
 accountInactive                        boolean,                -- True if the owner player's login account has become inactive.
 con                                    boolean,                -- true if this demiavatar thinks he has the con
 log                                    bytea,                  -- Captain's Log
 logtime                                timestamp,              -- when last send log to chain of command
 selfie                                 bytea,                  -- Selfie saved from inventory window.
 posture                                smallint,               -- CITIZEN_POSTURE enum value, from dbdef.h, when AI controlled.
 autoEat                                boolean,                -- true to automatically eat food.
 domain                                 bytea,                  -- AuDomain
 lastOnLine                             timestamp               -- Last time SetOnLine was called.
) INHERITS (a_Creature);
GRANT ALL ON a_Person TO audog;
CREATE INDEX a_Person_id_index ON a_Person USING btree (id);
CREATE INDEX a_Person_idp_index ON a_Person USING btree (idp);
SELECT * FROM a_Person;

-- An Item atom represents an instance of an inventory item, which is a quantity of a commodity, in the world.
--DROP TABLE a_Item;
CREATE TABLE a_Item
(
 atRest              boolean,         -- true while at rest
 vecX                float8,          -- Lateral speed in fps
 vecY                float8,          -- Forward speed in fps
 vecZ                float8,          -- Vertical speed in fps
 item                bytea,           -- DeInventoryItem
 ownerId             bigint,          -- Id of atom who 'set' this item, for detonated and fused items.
 ownerName           text,            -- Name of atom who 'set' this item, for detonated and fused items.
 ownerEmpire         bigint,          -- Empire of atom who 'set' this item, for detonated and fused items.
 cid                 integer,
 staticDisplay       boolean,         -- true if this item is on static display
 staticRoom          text             -- bd or sc room name where static displayer was standing
) INHERITS (a_Atom);
GRANT ALL ON a_Item TO audog;
CREATE INDEX a_Item_id_index ON a_Item USING btree (id);
CREATE INDEX a_Item_idp_index ON a_Item USING btree (idp);
SELECT * FROM a_Item;

-- DaDesignEnvironmnent
-- Environment in which old style spacecraft and buildings are designed, the ones that use n_Room and n_Detail records.
--DROP TABLE a_Arena CASCADE;
CREATE TABLE a_Arena
(
 matchState                             smallint,               -- 0=Prep, 1=Play, 2=Aftermath
 matchType                              smallint,               -- 0=Duel, 1=Solo
 maximizeShipTL                         boolean,                -- Make all ship designs at maxShipTL.
 maxShipTL                              smallint,               -- Maximum TL of ship designs allowed.
 name                                   text,                   -- Name of the arena, e.g. Person's name.
 opponentDesignsOk                      boolean,                -- False if everyone must use shipDesign.
 password                               text,                   -- Password for entry into the arena.
 shipDesign                             bigint,                 -- Design id if all player must use the same one, default otherwise.
 designIsSc                             boolean,                -- true if shipDesign is in n_ScDesign.
 spawnCrew                              smallint,               -- Spawn crew aboard ships: 0=no, 1=initial, 2=respawn
 spawnOfficer                           smallint,               -- Spawn officer aboard ships: 0=no, 1=initial, 2=respawn
 spawnPassengers                        smallint,               -- Spawn passengers aboard ships: 0=no, 1=initial, 2=respawn
 spawnTroops                            smallint,               -- Spawn troops aboard ships: 0=no, 1=initial, 2=respawn
 spawnVehicles                          smallint,               -- Spawn vehicles aboard ships: 0=no, 1=initial, 2=respawn
 teamShips                              smallint,               -- Initial ships per team.
 team1Ships                             smallint,               -- Ships remaining to spawn for team 1.
 team2Ships                             smallint                -- Ships remaining to spawn for team 2.
) INHERITS (a_Atom);
GRANT ALL ON a_Arena TO audog;
CREATE INDEX a_Arena_id_index ON a_Arena USING btree (id);
CREATE INDEX a_Arena_idp_index ON a_Arena USING btree (idp);
SELECT * FROM a_Arena;

-- DaDesigner
-- Environment in which old style spacecraft and buildings are designed, the ones that use n_Room and n_Detail records.
--DROP TABLE a_Designer CASCADE;
CREATE TABLE a_Designer
(
  archid                      bigint,                       -- architect id of loaded design, not necessarily the studio owner
  archname                    text,                         -- architect name of loaded design, not necessarily the studio owner
  studioid                    integer,                      -- Panel Id that created this environment
  modified                    boolean,                      -- true if design modified since last load or save
  designMode                  smallint,                     -- 0=Invalid, 1=space ship, 2=building
  design                      bytea,                        -- AuDesign
  ownerid                     bigint,                       -- owner id of the design studio instance, not necessarily the architect
  viewOnly                    boolean                       -- true if design cannot be copied to clipboard or saved.
) INHERITS (a_Atom);
GRANT ALL ON a_Designer TO audog;
CREATE INDEX a_Designer_id_index ON a_Designer USING btree (id);
CREATE INDEX a_Designer_idp_index ON a_Designer USING btree (idp);
SELECT * FROM a_Designer;

-- Design plane is the rounding grid inside the a_DesignEnv.
--DROP TABLE a_DesignPlane;
CREATE TABLE a_DesignPlane
(
  onBits                      smallint,                     -- b0=0,b1=45,b2=90,b3=135
  owner                       bigint,                       -- owner person atom
  render                      boolean,                      -- true=solid, false=lines
  roomid                      smallint,                     -- Current room id
  gravity                     boolean,                      -- true if gravity is enabled
  roomMode                    smallint,                     -- 0=draw room, 1=change room type
  inc                         float8,                       -- Grid increment.
  rad                         float8                        -- Grid radius.
) INHERITS (a_Atom);
GRANT ALL ON a_DesignPlane TO audog;
CREATE INDEX a_DesignPlane_id_index ON a_DesignPlane USING btree (id);
CREATE INDEX a_DesignPlane_idp_index ON a_DesignPlane USING btree (idp);
SELECT * FROM a_DesignPlane;

-- A building is a stationary unit, similar to a vessel.
--DROP TABLE a_Bd;
CREATE TABLE a_Bd
(
 airPressures       bytea,
 designId           bigint,             -- id of n_BdDesign.
 doorStates         bytea,
 bank               float8 default 0,   -- Bank account balance (DEPRECATED)
 banka              bytea,              -- Bank account balance with bank activity.
 baySetting         bytea,              -- weapon bay settings. which ids with power for weapons and commodity id for harvesters
 broker             bytea,              -- Broker settings
 capacitor          float4,             -- Capacitor level
 capitol            bigint,             -- Atom id of capitol building, if any. This makes me part of a city.
 cargo              bytea,              -- deprecated because it contains old dna
 cargo4             bytea,              -- New version contains species
 citizenDNA         bytea,              -- DNA of local populace. Deprecated. Replaced by dna4.
 resSpec            bytea,              -- species of residents
 cityName           text,               -- Name of city if I am a capitol building.
 cityPop            integer,            -- Total resident population of the city, troop population of bases.
 damage             bytea,              -- AuSpacecraftDamage
 designRpt          bytea,
 fence              bytea,              -- Fence and wall properties.
 games              bytea,              -- Games in progress at game tables.
 hp                 smallint,           -- Hit points remaining
 industry           smallint,           -- INDUSTRYID
 landingSystems     bytea,
 lastVisit          timestamp,          -- when last visited by a member of my empire, for loyalty decay
 launchers          bytea,              -- Space vehicle launchers
 lifts              bytea,
 log                bytea,              -- Event Log
 mfgproc            bytea,              -- DeManufacturingProcessArray
 ownerId            bigint,             -- Id of atom who owns this ship, for privately owned buildings.
 ownerName          text,               -- Name of atom who owns this ship, for privately owned buildings.
 platform           bigint,             -- Atom id of platform on which this building was placed.
 qual               smallint,           -- quality at which the building was manufactured
 reportHist         bytea,              -- City Report history, only at capitol building.
 reportTime         timestamp,          -- when capitol last recorded a city report
 roomLights         bytea,
 scFactory          bytea,              -- Spacecraft factory properties.
 shield             float4,             -- 0..1 ratio of available shields allocated. shield allocation fields all add up to 1
 systems            bytea,              -- auShipSystemArray
 transporters       bytea,
 turretPos          bytea
) INHERITS (a_Unit);
GRANT ALL ON a_Bd TO audog;
CREATE INDEX a_Bd_id_index ON a_Bd USING btree (id);
CREATE INDEX a_Bd_idp_index ON a_Bd USING btree (idp);
SELECT * FROM a_Bd;

-- An asteroid atom is a planet killer.
--DROP TABLE a_Asteroid;
CREATE TABLE a_Asteroid
(
 atRest              boolean,         -- true while at rest
 hits                integer,         -- hit points remaining
 size                smallint,        -- size 1 to 4
 spinAxis1X          float8,          -- Spin axis unit vector.
 spinAxis1Y          float8,          -- Spin axis unit vector.
 spinAxis1Z          float8,          -- Spin axis unit vector.
 spinSpeed1          float,           -- Spin speed in radians per sec.
 spinAxis2X          float8,          -- Spin axis unit vector.
 spinAxis2Y          float8,          -- Spin axis unit vector.
 spinAxis2Z          float8,          -- Spin axis unit vector.
 spinSpeed2          float,           -- Spin speed in radians per sec.
 targetId            bigint,          -- Id of target world atom.
 targetTime          timestamp,       -- Time of scheduled impact at the target.
 releaseTime         timestamp,       -- Time of release to approach the target.
 vecX                float8,          -- Lateral speed in fps
 vecY                float8,          -- Forward speed in fps
 vecZ                float8           -- Vertical speed in fps
) INHERITS (a_Atom);
GRANT ALL ON a_Asteroid TO audog;
CREATE INDEX a_Asteroid_id_index ON a_Asteroid USING btree (id);
CREATE INDEX a_Asteroid_idp_index ON a_Asteroid USING btree (idp);
SELECT * FROM a_Asteroid;
