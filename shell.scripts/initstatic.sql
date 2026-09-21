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

-- NOTE: Static data can be updated into the universe database without harming
--       the game data. All the scene master, manager and worker servers should
--       be restarted when this is done.

--       To update an existing universe database...
--         psql -f initstatic.sql auUniverse -U aumaster > log 2>&1
--          or remotely...
--         psql -h huey -f initstatic.sql auUniverse -U aumaster > log 2>&1

---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
----- Economy -----

-- This data describes all objects that can exist as a_Items.
-- Commodities are the core components of the economy.
-- class DbCommodity
DROP TABLE b_Commodity;
CREATE TABLE b_Commodity
(
  id               integer primary key unique,
  ammo1Qty         smallint DEFAULT 0,   -- Round count held for weapons that use ammo or have a charge.
  ammo1Type        smallint DEFAULT 0,   -- AMMOTYPE of ammo needed for weapon or provided by ammo.
  ammo2Qty         smallint DEFAULT 0,   -- Round count held for weapons that use ammo or have a charge.
  ammo2Type        smallint DEFAULT 0,   -- AMMOTYPE of ammo needed for weapon or provided by ammo.
                                         -- Not used for ammo because one unit reloads a weapon completely.
  armorEffectAbsorbAcid                 smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorEffectAbsorbBludgeon             smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorEffectAbsorbCold                 smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorEffectAbsorbCrush                smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorEffectAbsorbDisease              smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorEffectAbsorbElectric             smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorEffectAbsorbHeat                 smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorEffectAbsorbImplant              smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorEffectAbsorbPierce               smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorEffectAbsorbPoison               smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorEffectAbsorbRadiation            smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorEffectAbsorbSlash                smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorEffectAbsorbSonic                smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorEffectAbsorbStun                 smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorEffectAbsorbSuffocate            smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorEffectBlockAcid                  smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorEffectBlockBludgeon              smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorEffectBlockCold                  smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorEffectBlockCrush                 smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorEffectBlockDisease               smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorEffectBlockElectric              smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorEffectBlockHeat                  smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorEffectBlockImplant               smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorEffectBlockPierce                smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorEffectBlockPoison                smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorEffectBlockRadiation             smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorEffectBlockSlash                 smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorEffectBlockSonic                 smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorEffectBlockStun                  smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorEffectBlockSuffocate             smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorModeAbsorbArea                   smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorModeAbsorbContact                smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorModeAbsorbProjectile             smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorModeAbsorbPsionic                smallint DEFAULT 0,     -- armor effect as a percent (0-100) of total damage absorbed
  armorModeBlockArea                    smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorModeBlockContact                 smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorModeBlockProjectile              smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorModeBlockPsionic                 smallint DEFAULT 0,     -- armor effect as a number of damage points blocked
  armorSpec                             smallint DEFAULT 0,     -- WEAPONEFFECT of specialty. Hardened vs this effect. Armor takes less (1/2) wear.
  bitsWear         integer,              -- Where this commodity can be worn/used.
  capht            smallint DEFAULT 0,   -- >0 for Containers. Height of
                                         -- storage space, 20max
  capwid           smallint DEFAULT 0,   -- >0 for Containers. Width of
                                         -- storage space, 12max
  combits          integer DEFAULT 0,    -- COMMODITYBITS from dbef.h. OR'd together
  food             smallint DEFAULT 0,   -- food value if one unit is consumed
                                         -- liquor value
  health           smallint DEFAULT 0,   -- health affect if one unit is consumed, or [1..health] when damage effect is in combits
  iconPath         text,                 -- Path to small square image, 32x32 pixels, png                     
  modelPath        text,                 -- Path to the model used for this item                                      
  insertHeight     float4 DEFAULT 0,     -- Height above insertion point when on the ground and up vector applied
                                         -- Used to determine the height it lays on the ground
  upVectorX        float4 DEFAULT 0,     -- X component of up vector
  upVectorY        float4 DEFAULT 0,     -- Y component of up vector
  upVectorZ        float4 DEFAULT 1,     -- Z component of up vector
  light            smallint DEFAULT 0,   -- light radius in five feet increments. i.e. value 4 = 20' light radius
  name             text,                 -- Short name suitable for lists.
  newcond          smallint default 1,   -- Condition points when new.
  sizeht           smallint default 0,   -- Height required in a container.
  sizewid          smallint default 0,   -- Width required in a container.
  stackLimit       smallint DEFAULT 1,   -- Maximum count in a stack.
  wpn1Mode         smallint DEFAULT 0,   -- WEAPONMODE enum value from dbdef.h
  wpn1Range        smallint DEFAULT 0,   -- Weapon maximum range in feet.
  wpn1Effect1      smallint DEFAULT 0,   -- WEAPONEFFECT enum value. WE_None = Not a weapon for ammoType and ammoQty.
  wpn1Damage1      smallint DEFAULT 0,   -- Weapon damage dice roll [1..weaponDamage]
  wpn1DamageMod1   smallint DEFAULT 0,   -- Added to weaponDamage dice roll. LIMITED TO AN UNSIGNED BYTE.
  wpn1Radius1      smallint DEFAULT 0,   -- Weapon effect radius in feet.
  wpn1AP1          smallint DEFAULT 0,   -- Armor Pierce percent (0-100)
  wpn1ST1          smallint DEFAULT 0,   -- Strike Through amount (0-n)
  wpn1Effect2      smallint DEFAULT 0,   -- WEAPONEFFECT enum value.
  wpn1Damage2      smallint DEFAULT 0,   -- Weapon special damage [1..weaponDamage]
  wpn1DamageMod2   smallint DEFAULT 0,   -- Added to weapon 1..weaponDamage dice roll. LIMITED TO AN UNSIGNED BYTE.
  wpn1Radius2      smallint DEFAULT 0,   -- Weapon effect radius in feet.
  wpn1AP2          smallint DEFAULT 0,   -- Armor Pierce percent (0-100)
  wpn1ST2          smallint DEFAULT 0,   -- Strike Through amount (0-n)
  wpn2Mode         smallint DEFAULT 0,   -- WEAPONMODE enum value from dbdef.h
  wpn2Range        smallint DEFAULT 0,   -- Weapon maximum range in feet.
  wpn2Effect1      smallint DEFAULT 0,   -- WEAPONEFFECT enum value. WE_None = Not a weapon for ammoType and ammoQty.
  wpn2Damage1      smallint DEFAULT 0,   -- Weapon damage dice roll [1..weaponDamage]
  wpn2DamageMod1   smallint DEFAULT 0,   -- Added to weaponDamage dice roll. LIMITED TO AN UNSIGNED BYTE.
  wpn2Radius1      smallint DEFAULT 0,   -- Weapon effect radius in feet.
  wpn2AP1          smallint DEFAULT 0,   -- Armor Pierce percent (0-100)
  wpn2ST1          smallint DEFAULT 0,   -- Strike Through amount (0-n)
  wpn2Effect2      smallint DEFAULT 0,   -- WEAPONEFFECT enum value.
  wpn2Damage2      smallint DEFAULT 0,   -- Weapon special damage [1..weaponDamage]
  wpn2DamageMod2   smallint DEFAULT 0,   -- Added to weapon 1..weaponDamage dice roll. LIMITED TO AN UNSIGNED BYTE.
  wpn2Radius2      smallint DEFAULT 0,   -- Weapon effect radius in feet.
  wpn2AP2          smallint DEFAULT 0,   -- Armor Pierce percent (0-100)
  wpn2ST2          smallint DEFAULT 0,   -- Strike Through amount (0-n)
  wt               float4 DEFAULT 0      -- In pounds per single unit when carried in a person's inventory. Not relevant otherwise.
                                         -- 1 kg = 2.20462 lbs
);
GRANT ALL ON b_Commodity TO audog;
-- Fully Populated Here.
-- Add new items at the end.
-- New items probably also need information in the manufacturing processes
-- table to describe how they are made.
-- Projectile weapons define wRange,ammoType,ammoQty.
-- Projectile weapons do not define wEffect1,wDamage1,wDamageMod1,wRadius1.
-- Those values come from the ammo that is loaded.
-- Ammo defines wEffect1,wDamage1,wDamageMod1,wRadius1. One unit of
-- ammo always fully loads the weapon.
-- When ammo and a weapon define to-hit mods, they both apply.
-- When ammo type is 'launcher', the ammo determines the range. i.e. rockets and
-- missiles.
-- wEffect2, wDamage2, wDamageMod2, wRadius2 are defined when manufactured by
-- specialty of industry.
-- Contact weapons may have a range to extend the reach when wielded.
-- Energy weapons do not get weapon effects from the ammo.
-- When defining Clothing and Armor, bitsWear is currently limited to a few values:
--  Clothing =  IP_ClothingArms|IP_ClothingLegs|IP_ClothingLowerTorso|IP_ClothingUpperTorso = 256+2048+4096+8192 = 14592
--  Armor = IP_ArmorArms|IP_ArmorFeet|IP_ArmorHands|IP_ArmorLegs|IP_ArmorLowerTorso|IP_ArmorUpperTorso = 1+4+8+32+64+128 = 237
--  Helmet = IP_ArmorHead = 16 = 16
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (1,'Flag',':data/c_Flag.png',':data/mFlag.3dsX',0.0,0,0,1,8192+1,1,1,.5);
-- id=2 is the old computer that was not a storage device. DO NOT REUSE THIS ID.
INSERT INTO b_Commodity (id,name,iconPath) VALUES (3,'Electricity',':data/c_Electricity.png');
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, wpn1Mode,wpn1Range,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (4,'Hammer',':data/c_Hammer.png',':data/mHammer.3dsN',0.05,0,1,0,33554432+8192+1,1,2,1, 5, 2,1,8,11,4);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (5,'Air',':data/c_Air.png',':data/mAir.3dsN',1.8,0,0,1,8192+1,2,3,5);
INSERT INTO b_Commodity (id,name,iconPath,modelPath) VALUES (6,'Helicopter',':data/c_Helicopter.png',':data/mHelicopter.3dsN');
INSERT INTO b_Commodity (id,name,iconPath,modelPath) VALUES (7,'Space Rocket',':data/c_SpaceRocket.png',':data/mRocketShip.3dsX');
INSERT INTO b_Commodity (id,name,iconPath,modelPath) VALUES (8,'Space Fighter F1',':data/c_SpaceFighter.png',':data/mSpacefighter.3dsN');
INSERT INTO b_Commodity (id,name,iconPath,modelPath) VALUES (9,'Space Transport',':data/c_CommercialAircraft.png',':data/mFlyingAT.3dsN');
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (10,'Pistol Ammunition',':data/c_PistolAmmo.png',':data/mPistolAmmo.3dsX',0.05,0,0,1,8192+1,1,1,.2, 4,13,31,9);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1,wpn1AP1)
  VALUES (11,'Heavy Arms Ammunition Pi',':data/c_HeavyArmsAmmo.png',':data/mHeavyArmsAmmo.3ds',0.05,0,1,0,8192+1,1,5,1, 2,13,71,29,10,1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, wpn1Mode,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1) VALUES (12,'Bomb Bl Black Cat',':data/c_Bomb.png',':data/mBomb.3dsX',0.0,0,0,1,8192+1,2,8,250, 3,8,751,249,100);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, wpn1Mode,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1) VALUES (13,'Plastic Explosive',':data/c_PlasticExplosive.png',':data/mPlasticExplosive.3dsN',0.0,0,0,1,8192+1,2,2,2.2, 3,8,151,49,20);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (14,'Animal Carcass',':data/c_Animal.png',':data/mAnimal.3dsX',0.0,0,0,1,4096+3,4,6,10, 20,5);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (15,'Beans',':data/c_Beans.png',':data/mBeans.3dsN',0.3,0,0,1,20480+3,1,1,1, 20,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (16,'Beer',':data/c_Beer.png',':data/mBeer.3dsX',0.0,0,0,1,20480+5,1,1,1, 1,4);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (17,'Leather Armor',':data/c_LeatherBodyArmor.png',':data/mLeatherBodyArmor.3dsN',0.1,0,0,1,8192+1,3,4,2, 8, 10,237);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (18,'Bread',':data/c_Bread.png',':data/mBread.3dsX',0.0,0,0,1,20480+3,1,1,1, 15,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (19,'Candy',':data/c_Candy.png',':data/mCandy.3dsX',0.0,0,0,1,20480+3,1,1,1, 20,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (20,'Cheese',':data/c_Cheese.png',':data/mCheese.3dsX',0.0,0,0,1,20480+3,1,1,1, 20,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (21,'Jeans and Shirt',':data/c_JeansShirt.png',':data/mLinenShirt.3dsN',0.0,0,0,1,8192+1,2,2,.5, 0, 3,14592);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (22,'Coal',':data/c_Coal.png',':data/mCoal.3dsX',0.0,0,0,1,1,2,2,10);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (23,'Oil',':data/c_CrudeOil.png',':data/mCrudeOil.3dsN',0.0,0,0,1,8192+1,3,3,10);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (24,'Crystals',':data/c_Crystals.png',':data/mCrystals.3dsX',0.0,0,0,1,1,2,2,10);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, wpn1Mode,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1) VALUES (25,'Dynamite',':data/c_Dynamite.png',':data/mDynamite.3dsX',0.0,0,0,1,8192+1,2,1,1.65, 4,8,151,49,20);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (26,'Eggs',':data/c_Eggs.png',':data/mEggs.3dsX',0.0,0,0,1,20480+3,1,1,.5, 15,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (27,'Electronic Part',':data/c_ElectronicPart.png',':data/mElectronicPart.3dsX',0.0,0,0,1,8192+1,2,1,.25);
INSERT INTO b_Commodity (id,name,iconPath,modelPath) VALUES (28,'Farm Machinery',':data/c_FarmMachinery.png',':data/mFarmEquipment.3dsN');
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (29,'Fertilizer',':data/c_Fertilizer.png',':data/mFertilizer.3dsX',0.0,0,0,1,1,3,4,8);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (30,'Plant Fiber',':data/c_Fiber.png',':data/mFiber.3dsX',0.0,0,0,1,1,2,2,5);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,health, wpn1Mode,wpn1Range,ammo1Type,ammo1Qty) VALUES (31,'Pistol',':data/c_Pistol.png',':data/mPistol.3dsN',0.1,0,1,0,8192+256+1,1,1,2, 80,6, 5,150,4,15);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,health, wpn1Mode,wpn1Range,ammo1Type,ammo1Qty) VALUES (32,'Rifle',':data/c_Rifle.png',':data/mRifle.3dsN',0.05,0,1,0,8192+256+1,2,6,4, 100,10, 5,1000,5,30);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (33,'Fish',':data/c_Fish.png',':data/mFish.3dsN',0.15,1,0,0,4096+3,2,1,1, 15,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (34,'Flour',':data/c_Flour.png',':data/mFlour.3dsX',0.0,0,0,1,4096+3,1,2,1, 15,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (35,'Fruit',':data/c_Fruit.png',':data/mFruit.3dsX',0.0,0,0,1,20480+3,1,1,1, 10,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (36,'Gasoline',':data/c_Gasoline.png',':data/mGasoline.3dsN',2.0,0,0,1,8192+1,2,3,10);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (37,'Gems',':data/c_Gems.png',':data/mGems.3dsN',0.4,0,0,1,1,1,1,.1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (38,'Grain',':data/c_Grain.png',':data/mGrain.3dsN',0.0,0,0,1,20480+3,2,3,1, 15,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (39,'Grapes',':data/c_Grapes.png',':data/mGrapes.3dsN',0.1,1,0,0,20480+3,1,1,1, 10,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (40,'Gun Powder',':data/c_GunPowder.png',':data/mGunPowder.3dsN',1.0,0,0,1,1,2,2,1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (41,'Flak Helmet',':data/c_WovenHelmet.png',':data/mWovenHelmet.3dsN',0.0,0,0,1,8192+1,2,2,.5, 13, 8,16);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (42,'Herbs',':data/c_Herbs.png',':data/mHerbs.3dsN',0.0,0,0,1,20480+3,1,1,1, 10,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (43,'Hops',':data/c_Hops.png',':data/mHops.3dsN',1.45,0,0,1,20480+3,2,2,1, 10,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2) VALUES (44,'Hydrogen',':data/c_Hydrogen.png',':data/mHydrogen.3dsN',2.5,0,0,1,8192+1,2,3,10, 9,8,31,19,10, 12,21,9,15);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (45,'Ice',':data/c_Ice.png',':data/mIce.3dsN',0.0,0,0,1,1,2,2,10);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,bitsWear) VALUES (46,'Crystal Necklace',':data/c_CrystalJewelry.png',':data/mCrystalJewelry.3dsN',0.1,0,1,0,8192+1,1,1,.3, 1,524288);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (47,'Leather',':data/c_Leather.png',':data/mLeather.3dsN',0.1,1,0,0,20480+3,3,5,10, 15,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, health) VALUES (48,'Lumber',':data/c_Lumber.png',':data/mLumber.3dsN',0.1,0,1,0,8192+257,1,8,10, 15);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (49,'Animal Meat',':data/c_AnimalMeat.png',':data/mAnimalMeat.3dsX',0.0,0,0,1,20480+3,1,1,1, 20,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (50,'Mechanical Part',':data/c_MechanicalPart.png',':data/mMechanicalPart.3dsN',0.15,0,-1,0,8192+1,2,2,5);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, health) VALUES (51,'Metal',':data/c_Metal.png',':data/mMetal.3dsN',0.1,1,0,0,8192+257,2,1,10, 15);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (52,'Milk',':data/c_Milk.png',':data/mMilk.3dsN',0.55,0,0,1,20480+3,1,1,1, 20,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (53,'Minerals',':data/c_Minerals.png',':data/mMinerals.3dsN',0.0,0,0,1,1,2,2,10);
INSERT INTO b_Commodity (id,name,iconPath,modelPath) VALUES (54,'Mine Machinery',':data/c_MineMachinery.png',':data/mMineMachinery.3dsX');
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1,wpn1Range)
  VALUES (55,'Small Missile Bl Zipper',':data/c_MissileSm.png',':data/mMissileSm.3dsN',0.1,1,0,0,8192+1,1,8,50, 7,8,126,24,15,2500);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1,wpn1Range)
  VALUES (56,'Large Missile Bl Thumper',':data/c_MissileLg.png',':data/mMissileLg.3dsN',3.8,0,0,1,1,3,12,300, 3,8,701,199,90,25000);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2) VALUES (57,'Natural Gas',':data/c_NaturalGas.png',':data/mNaturalGas.3dsN',2.0,0,0,1,8192+1,2,3,10, 9,8,26,19,8, 12,16,9,13);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (58,'Nuts',':data/c_Nuts.png',':data/mNuts.3dsN',0.35,0,0,1,20480+3,1,1,1, 15,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (59,'Ore',':data/c_Ore.png',':data/mOre.3dsN',0.0,0,0,1,1,2,2,10);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (60,'Pastry',':data/c_Pastry.png',':data/mPastry.3dsN',0.0,0,0,1,20480+3,1,1,.5, 10,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2) VALUES (61,'Petrochemicals',':data/c_Petrochemicals.png',':data/mPetrochemicals.3dsN',0.0,0,0,1,1,3,3,10, 9,12,31,19,10, 1,21,9,15);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (62,'Pharmaceuticals',':data/c_Pharmaceuticals.png',':data/mPharmaceuticals.3dsN',0.3,0,0,1,8192+4096+1,1,1,.2, 1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (63,'Plastic',':data/c_Plastic.png',':data/mPlastic.3dsX',0.0,0,0,1,8192+1,1,6,10);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (64,'Large Rocket Motor',':data/c_RocketMotorLg.png',':data/mRocketMotorLg.3dsN',3.2,0,0,1,1,3,9,50);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear,light) VALUES (65,'Metal EV Suit',':data/c_PowerArmor.png',':data/mPowerArmor.3dsN',0.0,0,0,1,8192+1,4,10,50, 7, 80,237,15);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (66,'Radioactives',':data/c_Radioactives.png',':data/mRadioactives.3dsN',0.5,0,0,1,8192+1,2,2,10);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1,wpn1Range) VALUES (67,'Small Rocket Bl Partypopper',':data/c_RocketSm.png',':data/mRocketSm.3dsN',0.2,1,0,0,8192+1,1,8,40, 7,8,151,49,20,2500);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1,wpn1Range) VALUES (68,'Large Rocket Bl Thud',':data/c_RocketLg.png',':data/mRocketLg.3dsN',3.8,0,0,1,1,3,12,280, 3,8,751,249,100,25000);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (69,'Small Rocket Motor',':data/c_RocketMotorSm.png',':data/mRocketMotorSm.3dsN',0.15,1,0,0,1,1,6,25);
INSERT INTO b_Commodity (id,name,iconPath,modelPath) VALUES (70,'Commercial Ship',':data/c_CommercialShip.png',':data/mCommercialShip.3dsX');
INSERT INTO b_Commodity (id,name,iconPath,modelPath) VALUES (71,'Naval Ship',':data/c_NavalShip.png',':data/mNavalShip.3dsX');
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (72,'EV Helmet',':data/c_VacSuitHelmet.png',':data/mVacSuitHelmet.3dsN',0.0,0,0,1,8192+1,2,2,1, 0, 10,16);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,health) VALUES (73,'Medical Kit',':data/c_MedKit.png',':data/mMedKit.3dsN',0.9,0,0,1,8192+4096+129,2,2,1, 2,20);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (74,'Spices',':data/c_Spices.png',':data/mSpices.3dsN',0.4,0,0,1,20480+3,1,1,1, 10,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (75,'Rifle Ammunition',':data/c_RifleAmmo.png',':data/mRifleAmmo.3dsX',0.0,0,0,1,8192+1,1,1,.8, 5,13,46,14);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, health) VALUES (76,'Stone',':data/c_Stone.png',':data/mStone.3dsN',0.0,0,0,1,8192+257,2,1,10, 15);
INSERT INTO b_Commodity (id,name,iconPath,modelPath) VALUES (77,'Submarine',':data/c_Submarine.png',':data/mNeptune.3dsX');
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (78,'Sugar',':data/c_Sugar.png',':data/mSugar.3dsX',0.0,0,0,1,4096+3,1,2,1, 20,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (79,'Textiles',':data/c_Textiles.png',':data/mTextiles.3dsN',0.1,0,0,1,8192+1,1,5,10);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear,light) VALUES (80,'EV Suit',':data/c_VacSuit.png',':data/mVacSuit.3dsN',0.0,0,0,1,8192+1,3,8,25, 0, 25,237,15);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (81,'Vegetable',':data/c_Vegetable.png',':data/mVegetable.3dsN',0.0,0,0,1,20480+3,1,1,1, 10,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath) VALUES (82,'SUV',':data/c_Suv.png',':data/mSUV.3dsN');
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (83,'Water',':data/c_Water.png',':data/mWater.3dsN',1.5,0,0,1,20480+64+3,1,2,1, 0,4);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (84,'Whiskey',':data/c_Whiskey.png',':data/mWhiskey.3dsN',0.5,0,0,1,16384+5,1,2,1, 2,4);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (85,'Wine',':data/c_Wine.png',':data/mWine.3dsN',0.8,0,0,1,20480+5,1,2,1, 1,4);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, health) VALUES (86,'Log',':data/c_Wood.png',':data/mWood.3dsN',0.2,-1,0,0,8192+257,2,8,10, 15);
INSERT INTO b_Commodity (id,name,iconPath) VALUES (87,'Atmosphere Density',':data/c_Atmosphere.png');
INSERT INTO b_Commodity (id,name,iconPath) VALUES (88,'Water in the Environment',':data/c_Hydrographics.png');
INSERT INTO b_Commodity (id,name,iconPath) VALUES (89,'Vegetation Density',':data/c_Vegetation.png');
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (90,'Metal Armor',':data/c_MetalBodyArmor.png',':data/mMetalBodyArmor.3dsN',0.0,0,0,1,8192+1,3,4,10, 15, 40,237);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (91,'Plastic Armor',':data/c_PlasticBodyArmor.png',':data/mPlasticBodyArmor.3dsN',0.3,1,0,0,8192+1,3,4,4, 1, 30,237);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (92,'Flak Vest',':data/c_WovenBodyArmor.png',':data/mWovenBodyArmor.3dsN',0.15,0,0,1,8192+1,3,4,6, 13, 20,237);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, wpn1Mode,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1) VALUES (93,'Nitrate Explosive',':data/c_NitrateExplosive.png',':data/mNitrateExplosive.3dsN',0.0,0,0,1,8192+1,2,3,3, 3,8,151,49,20);
INSERT INTO b_Commodity (id,name,iconPath) VALUES (94,'Sunlight',':data/c_Sunlight.png');
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (95,'Leather Clothing',':data/c_LeatherClothes.png',':data/mLeatherShirt.3dsN',0.15,1,0,0,8192+1,2,3,1, 8, 5,14592);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (96,'Leather Helmet',':data/c_LeatherHelmet.png',':data/mLeatherHelmet.3dsN',0.0,0,0,1,8192+1,2,2,.6, 8, 6,16);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (97,'Plastic Helmet',':data/c_PlasticHelmet.png',':data/mPlasticHelmet.3dsN',0.0,0,0,1,8192+1,2,2,.7, 1, 10,16);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,bitsWear) VALUES (98,'Gold Bracelet',':data/c_MetalJewelry.png',':data/mMetalJewelry.3dsN',0.05,0,1,0,8192+1,1,1,.3, 2,1879048192);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,bitsWear) VALUES (99,'Gem Ring',':data/c_GemJewelry.png',':data/mGemJewelry.3dsN',0.1,0,1,0,8192+1,1,1,.1, 2,1089470464);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (100,'Fish Meat',':data/c_FishMeat.png',':data/mFishMeat.3dsN',0.15,0,0,1,20480+3,1,1,1, 15,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (101,'Vodka',':data/c_Vodka.png',':data/mVodka.3dsN',0.75,0,0,1,16384+5,1,2,1, 2,4);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (102,'Heavy Weapon',':data/c_HeavyWeapon.png',':data/mTurretGun.3dsN',1.4,0,0,1,8192+1,3,20,100);
INSERT INTO b_Commodity (id,name,iconPath,modelPath) VALUES (103,'Tank',':data/c_Tank.png',':data/mTank.3dsX');
INSERT INTO b_Commodity (id,name,iconPath,modelPath) VALUES (104,'APC',':data/c_APC.png',':data/mAT.3dsN');
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (105,'Metal Helmet',':data/c_MetalHelmet.png',':data/mMetalHelmet.3dsN',0.0,0,0,1,8192+1,2,2,1, 15, 12,16);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (106,'Metal EV Helmet',':data/c_PowerArmorHelmet.png',':data/mPowerArmorHelmet.3dsN',0.0,0,0,1,8192+1,2,2,1, 7, 25,16);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, capwid,capht) VALUES (107,'Crate',':data/c_Crate.png',':data/mCrate.3dsN',0.0,0,0,1,8192+1,12,20,1, 50, 12,20);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, light) VALUES (108,'Lantern',':data/c_Lantern.png',':data/mLantern.3dsN',0.85,0,0,1,8192+65,1,2,1, 10, 10);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, capwid,capht) VALUES (109,'Knapsack',':data/c_Knapsack.png',':data/mKnapsack.3dsN',0.0,0,-1,0,8192+1,4,5,.5, 5, 4,5);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, capwid,capht) VALUES (110,'Backpack',':data/c_Backpack.png',':data/mBackpack.3dsN',0.0,1,0,0,8192+1,6,8,1, 10, 6,8);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (111,'Lumenite',':data/c_Lumenite.png',':data/mLumenite.3dsN',0.5,0,0,1,8192+1,2,2,10);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (112,'Blank Disk',':data/c_BlankDisk.png',':data/Disk.SoH',0.05,1,0,0,8192+1,1,1,.22);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (113,'Refining Antiflux Particles from Atmosphere Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, wpn1Mode,wpn1Range,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (114,'Shovel',':data/c_Shovel.png',':data/mShovel.3dsN',0.1,1,0,0,536870912+8192+1,1,8,2, 5, 2,3,15,11,4);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (115,'Gin',':data/c_Gin.png',':data/mGin.3dsN',0.65,0,0,1,16384+5,1,2,1, 2,4);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, wpn1Mode,wpn1Range,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (116,'Knife',':data/c_Knife.png',':data/mKnife.3dsN',0.02,0,-1,0,268435456+12288+1,1,2,.8, 100, 2,1,13,16,4);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,health, wpn1Mode,wpn1Range,ammo1Type,ammo1Qty) VALUES (117,'Shotgun',':data/c_Shotgun.png',':data/mShotgun.3dsN',0.1,0,-1,0,8192+256+1,2,8,3.5, 90,10, 5,150,6,8);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (118,'Shotgun Ammunition',':data/c_ShotgunAmmo.png',':data/mShotgunAmmo.3dsX',0.0,0,0,1,8192+1,1,1,1, 6,8,21,9);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (119,'Antidote',':data/c_Antidote.png',':data/mAntidote.3dsX',0.0,0,0,1,12288+33,1,1,.2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (120,'Antitoxin',':data/c_Antitoxin.png',':data/mAntitoxin.3dsX',0.0,0,0,1,12288+17,1,1,.2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (121,'Rum',':data/c_Rum.png',':data/mRum.3dsN',0.8,0,0,1,16384+5,1,2,1, 2,4);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (122,'Tequila',':data/c_Tequila.png',':data/mTequila.3dsN',0.7,0,0,1,16384+5,1,2,1, 2,4);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, wpn1Mode,wpn1Range,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (123,'Fitting Wrench',':data/c_FittingWrench.png',':data/mFittingWrench.3dsN',0.05,0,-1,0,1048576+8192+1,1,1,.5, 5, 2,1,8,6,0);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, wpn1Mode,wpn1Range,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (124,'Glue Gun',':data/c_GlueGun.png',':data/mGlueGun.3dsN',0.05,0,-1,0,67108864+8192+1,1,1,.5, 5, 2,1,3,8,0);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, wpn1Mode,wpn1Range,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (125,'Screw Driver',':data/c_ScrewDriver.png',':data/mScrewDriver.3dsN',0.05,1,0,0,2097152+8192+1,1,1,.5, 5, 2,1,13,8,0);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, wpn1Mode,wpn1Range,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (126,'Sewing Needle',':data/c_SewingNeedle.png',':data/mSewingNeedle.3dsN',0.1,0,0,1,134217728+8192+1,1,1,.1, 4, 2,1,13,4,0);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, wpn1Mode,wpn1Range,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (127,'Soldering Iron',':data/c_SolderIron.png',':data/mSolderingIron.3dsN',0.05,1,0,0,4194304+8192+1,1,1,.25, 5, 2,1,12,8,0);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, wpn1Mode,wpn1Range,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (128,'Tongs',':data/c_Tongs.png',':data/mTongs.3dsN',0.1,0,-1,0,8388608+8192+1,2,2,1, 5, 2,1,8,9,1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, wpn1Mode,wpn1Range,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (129,'Wrench',':data/c_Wrench.png',':data/mWrench.3dsN',0.05,0,1,0,16777216+8192+1,1,2,1, 5, 2,1,8,9,1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (130,'Hay',':data/c_Hay.png',':data/mHay.3dsN',2.0,0,0,1,20480+3,6,3,2, 10,5);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, light, wpn1Mode,wpn1Range,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (131,'Torch',':data/c_Torch.png',':data/mTorch.3dsN',0.1,1,0,0,8192+65,1,3,1, 5, 7, 2,2,12,7,1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath) VALUES (132,'Sailboat',':data/c_SailBoat.png',':data/mWoodenBoat.3dsX');
INSERT INTO b_Commodity (id,name,iconPath,modelPath) VALUES (133,'Motor Bike',':data/c_MotorBike.png',':data/mMotorBike.3dsN');
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (134,'Eludium',':data/c_Eludium.png',':data/mEludium.3dsN',0.05,1,0,0,1,1,2,.1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (135,'Pliers',':data/c_Pliers.png',':data/mPliers.3dsN',0.02,0,1,0,1073741824+8192+1,1,2,.5, 6);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, wpn1Mode,wpn1Range,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (136,'Torque Wrench',':data/c_TorqueWrench.png',':data/mTorqueWrench.3dsN',0.04,1,0,0,524288+8192+1,1,3,1, 3, 2,1,8,9,1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (137,'Vulcanite',':data/c_Vulcanite.png',':data/mVulcanite.3dsN',0.1,0,0,1,1,2,2,9);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (138,'Ioplasma',':data/c_Ioplasma.png',':data/mIoplasma.3dsN',0.0,0,0,1,1,2,3,8);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (139,'Magmex',':data/c_Magmex.png',':data/mMagmex.3dsN',0.0,0,0,1,1,2,2,8);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2) VALUES (140,'Phlogiston',':data/c_Phlogiston.png',':data/mPhlogiston.3dsN',0.0,0,0,1,1,2,3,4, 9,8,21,19,10, 12,21,19,15);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (141,'Bolite',':data/c_Bolite.png',':data/mBolite.3dsN',0.0,0,0,1,1,2,2,20);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (142,'Cryozine',':data/c_Cryozine.png',':data/mCryozine.3dsN',3.0,0,0,1,1,1,2,1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (143,'Myrathane',':data/c_Myrathane.png',':data/mMyrathane.3dsN',0.9,0,0,1,1,2,3,9);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (144,'Polytaride',':data/c_Polytaride.png',':data/mPolytaride.3dsN',1.8,0,0,1,1,2,3,9);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (145,'Vulcium',':data/c_Vulcium.png',':data/mVulcium.3dsN',0.1,1,0,0,8192+1,2,1,8);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (146,'Magmium',':data/c_Magmium.png',':data/mMagmium.3dsN',0.1,1,0,0,8192+1,2,1,6);
INSERT INTO b_Commodity (id,name,iconPath) VALUES (147,'Lava in the Environment',':data/c_Magma.png');
INSERT INTO b_Commodity (id,name,iconPath) VALUES (148,'Supercooled Liquid in the Environment',':data/c_SupercooledLiquid.png');
INSERT INTO b_Commodity (id,name,iconPath) VALUES (149,'Inferno Atmosphere Density',':data/c_InfernoAtmosphere.png');
INSERT INTO b_Commodity (id,name,iconPath) VALUES (150,'Frigid Atmosphere Density',':data/c_FrigidAtmosphere.png');
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, wpn1Mode,wpn1Range,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (151,'Magmium Knife',':data/c_MagmiumKnife.png',':data/mKnifeM.3dsN',0.02,0,-1,0,268435456+12288+1,1,2,.65, 150, 2,1,13,21,4);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, wpn1Mode,wpn1Range,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (152,'Vulcium Knife',':data/c_VulciumKnife.png',':data/mKnifeV.3dsN',0.02,0,-1,0,268435456+12288+1,1,2,.5, 200, 2,1,13,26,4);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear,light) VALUES (153,'Magmium EV Suit',':data/c_MagmiumPowerArmor.png',':data/mPowerArmorM.3dsN',0.0,0,0,1,8192+1,4,10,30, 12, 90,237,15);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (154,'Magmium EV Helmet',':data/c_MagmiumPowerArmorHelmet.png',':data/mPowerArmorHelmetM.3dsN',0.0,0,0,1,8192+1,2,2,.5, 12, 27,16);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear,light) VALUES (155,'Vulcium EV Suit',':data/c_VulciumPowerArmor.png',':data/mPowerArmorV.3dsN',0.0,0,0,1,8192+1,4,10,10, 5, 100,237,15);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (156,'Vulcium EV Helmet',':data/c_VulciumPowerArmorHelmet.png',':data/mPowerArmorHelmetV.3dsN',0.0,0,0,1,8192+1,2,2,.1, 5, 30,16);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (157,'Cronodollars',':data/c_Money.png',':data/mMoney.3dsX',0.0,0,0,1,0,1,1,.0625);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (158,'Refining Borexino Precipitate from Atmosphere Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (159,'Gold Ore',':data/c_GoldOre.png',':data/GoldOre.SoH',0.0,0,0,1,1,2,2,15);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (160,'Gold',':data/c_Gold.png',':data/Gold.SoH',0.1,1,0,0,8192+257,2,1,2.20462);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (161,'Computer',':data/c_Computer.png',':data/mComputer.3dsN',0.0,0,0,1,8192+1,1,2,.5, 40);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,bitsWear, armorModeAbsorbPsionic,armorModeBlockPsionic) VALUES (162,'Gold Ring',':data/c_GoldRing.png',':data/GoldRing.SoH',0.1,0,1,0,8192+1,1,1,.1, 2,1089470464, 8,10);

INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (226,'Grav Coupling',':data/c_GravCoupling.png',':data/mGravCoupling.3dsN',0.4,0,0,1,8192+1,2,2,.5);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (227,'Cryo Heat Sink',':data/c_CryoHeatSink.png',':data/mCryoHeatSink.3dsN',0.32,0,0,-1,8192+1,1,1,.25);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (228,'Io Tube',':data/c_IoTube.png',':data/mIoTube.3dsN',0.3,1,0,0,8192+1,1,2,.15);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, wpn1Mode,wpn1Range,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (229,'Clamp',':data/c_Clamp.png',':data/mClamp.3dsN',0.04,0,1,0,131072+8192+1,1,1,1, 4, 2,1,10,9,1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (230,'Oscilloscope',':data/c_OScope.png',':data/mOScope.3dsN',0.05,0,1,0,262144+8192+1,1,2,.5, 4);
INSERT INTO b_Commodity (id,name,iconPath,modelPath) VALUES (231,'Space Fighter F2',':data/c_SpaceFighter2.png',':data/mSpaceFighter2.3dsN');
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (232,'Mission Disk',':data/c_MissionDisk.png',':data/Disk.SoH',0.05,1,0,0,1,1,1,.22);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, health,food) VALUES (233,'Bone',':data/c_Bone.png',':data/mBone.3dsN',0.3,1,0,0,4096+259,1,3,3, 9,15);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (234,'Command And Control Module',':data/c_CommandAndControlModule.png',':data/ModuleCnC.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (235,'Gravity Drive Module',':data/c_DriveModule.png',':data/ModuleDrive.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (236,'Rocket Drive Module',':data/c_DriveModule.png',':data/ModuleDrive.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (237,'Metal Hull',':data/c_Metal.png',':data/ModuleHull.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (238,'Life Support Module',':data/c_LifeSupportModule.png',':data/ModuleLifeSupport.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (239,'Hydrogen Power Plant Module',':data/c_PowerPlantModule.png',':data/ModulePower.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (240,'Energy Shield Module',':data/c_ShieldModule.png',':data/ModuleShield.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (241,'Kinetic Shield Module',':data/c_ShieldModule.png',':data/ModuleShield.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (242,'Cloaking Shield Module',':data/c_ShieldModule.png',':data/ModuleShield.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (243,'Surgery Medical Module',':data/c_MedicalModule.png',':data/ModuleMedical.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (244,'Genetics Medical Module',':data/c_MedicalModule.png',':data/ModuleMedical.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (245,'Cybernetics Medical Module',':data/c_MedicalModule.png',':data/ModuleMedical.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (246,'Personnel Transporter Module',':data/c_TransporterModule.png',':data/ModuleTransporter.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (247,'Vehicle Transporter Module',':data/c_TransporterModule.png',':data/ModuleTransporter.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (248,'Energy Weapon Turret Module',':data/c_TurretModule.png',':data/ModuleTurret.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (250,'Weapon Launcher Turret Module',':data/c_TurretModule.png',':data/ModuleTurret.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (252,'Energy Weapon Bay Module',':data/c_BayModule.png',':data/ModuleBay.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (254,'Weapon Launcher Bay Module',':data/c_BayModule.png',':data/ModuleBay.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (256,'Atmosphere Condenser Bay Module',':data/c_BayModule.png',':data/ModuleBay.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (257,'Prospector Bay Module',':data/c_BayModule.png',':data/ModuleBay.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (258,'Tractor Beam Bay Module',':data/c_BayModule.png',':data/ModuleBay.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (259,'Wormhole P FTL Drive Module',':data/c_FTLModule.png',':data/ModuleFTL.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (260,'Warp 1 FTL Drive Module',':data/c_FTLModule.png',':data/ModuleFTL.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (261,'Ballistic Weapon Bay Module',':data/c_BayModule.png',':data/ModuleBay.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (262,'Ballistic Weapon Turret Module',':data/c_TurretModule.png',':data/ModuleTurret.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (263,'Sensor Module',':data/c_SensorModule.png',':data/ModuleSensor.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (264,'Magmium Heat Sink',':data/c_MagmiumHeatSink.png',':data/mCryoHeatSink.3dsN',0.32,0,0,-1,8192+1,1,1,.25);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (265,'Vulcium Heat Sink',':data/c_VulciumHeatSink.png',':data/mCryoHeatSink.3dsN',0.32,0,0,-1,8192+1,1,1,.25);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (266,'Myrasplicer',':data/c_Myrasplicer.png',':data/Myrasplicer.SoH',0,0,0,1,8192+1,1,2,1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (267,'Microscope',':data/c_Microscope.png',':data/Microscope.SoH',0,0,0,1,8192+1,1,2,.5, 4);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (268,'Gigacell',':data/c_Gigacell.png',':data/mGigacell.3ds',0,0,0,1,8192+1,1,1,.5, 1,12,61,19);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (269,'Tesloid Dissipator',':data/c_TesloidDissipator.png',':data/TesloidDissipator.SoH',0,0,0,1,8192+1,1,2,2.2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (270,'Nanopath Calibrator',':data/c_NanopathCalibrator.png',':data/NanopathCalibrator.SoH',0,0,0,1,8192+1,1,2,.22);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (271,'Antiflux Particles',':data/c_AntifluxParticles.png',':data/mAntifluxParticles.3dsN',0,0,0,1,1,2,2,.22);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (272,'Borexino Precipitate',':data/c_BorexinoPrecipitate.png',':data/BorexinoPrecipitate.SoH',0,0,0,1,1,2,2,2.2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (273,'Type O Preons',':data/c_TypeOPreons.png',':data/mPreon.3ds',0,0,0,1,1,2,2,1.1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (274,'Type B Preons',':data/c_TypeBPreons.png',':data/mPreon.3ds',0,0,0,1,1,2,2,1.1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (275,'Type A Preons',':data/c_TypeAPreons.png',':data/mPreon.3ds',0,0,0,1,1,2,2,1.1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (276,'Type F Preons',':data/c_TypeFPreons.png',':data/mPreon.3ds',0,0,0,1,1,2,2,1.1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (277,'Type G Preons',':data/c_TypeGPreons.png',':data/mPreon.3ds',0,0,0,1,1,2,2,1.1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (278,'Type K Preons',':data/c_TypeKPreons.png',':data/mPreon.3ds',0,0,0,1,1,2,2,1.1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (279,'Type M Preons',':data/c_TypeMPreons.png',':data/mPreon.3ds',0,0,0,1,1,2,2,1.1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (280,'Magmium Hull',':data/c_Magmium.png',':data/ModuleHull.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (281,'Vulcium Hull',':data/c_Vulcium.png',':data/ModuleHull.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (282,'Paper',':data/c_Paper.png',':data/mPaper.3dsN',0.0,1,0,0,8192+1,1,1,.01, 1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (283,'Particle Collector Bay Module',':data/c_BayModule.png',':data/ModuleBay.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (284,'DNA Sample Kit',':data/c_DNASampleKit.png',':data/mAntidote.3dsX',0.0,0,0,1,8192+4096+1,1,1,.2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (285,'DNA Sample',':data/c_DNASample.png',':data/mAntidote.3dsX',0.0,0,0,1,1,1,1,.2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (289,'Siphon Bay Module',':data/c_BayModule.png',':data/ModuleBay.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (290,'Officer Log',':data/c_OfficerLog.png',':data/Disk.SoH',0.05,1,0,0,1,1,1,.22);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (291,'Wildcatter Bay Module',':data/c_BayModule.png',':data/ModuleBay.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,health, wpn1Mode,wpn1Range,ammo1Type,ammo1Qty) VALUES (292,'Кала́шников',':data/c_AK47.png',':data/mAK47.3dsN',0.05,0,1,0,8192+256+1,2,6,4, 100,10, 5,1000,5,30);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, capwid,capht) VALUES (293,'Utility Pouch',':data/c_UtilityPouch.png',':data/mKnapsack.3dsN',0.0,0,-1,0,8192+1,5,3,.25, 4, 5,3);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, capwid,capht) VALUES (294,'Grav Pack',':data/c_GravPack.png',':data/mBackpack.3dsN',0.0,1,0,0,8192+1,8,12,2.2, 12, 8,12);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (295,'Silage',':data/c_Hay.png',':data/mHay.3dsN',2.0,0,0,1,20480+3,6,3,2, 10,5);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,health, wpn1Mode,wpn1Range,ammo1Type,ammo1Qty, wpn2Mode,wpn2Range,ammo2Type)
  VALUES (296,'Laser Rifle',':data/c_LaserRifle.png',':data/mLaserRifle.3ds',0.05,1,0,0,8192+256+1,2,6,4, 100,5, 5,1200,1,50, 5,1200,8);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type, wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2, wpn1Range)
  VALUES (297,'Small Rocket BlBu Firecracker',':data/c_RocketSm.png',':data/mRocketSm.3dsN',0.2,1,0,0,8192+1,1,8,44, 7, 8,151,49,20, 12,131,39,18, 2500);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type, wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2, wpn1Range)
  VALUES (298,'Large Rocket BlBu Molotov',':data/c_RocketLg.png',':data/mRocketLg.3dsN',3.8,0,0,1,1,3,12,330, 3, 8,751,249,100, 12,731,239,90, 25000);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type, wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2, wpn1Range)
  VALUES (299,'Small Missile BlBu Barnburner',':data/c_MissileSm.png',':data/mMissileSm.3dsN',0.1,1,0,0,8192+1,1,8,55, 7, 8,126,24,15, 12,106,14,13, 2500);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type, wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2, wpn1Range)
  VALUES (300,'Large Missile BlBu Fireball',':data/c_MissileLg.png',':data/mMissileLg.3dsN',3.8,0,0,1,1,3,12,350, 3, 8,701,199,90, 12,681,189,89, 25000);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type, wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2, wpn1Range)
  VALUES (301,'Small Rocket SlPi Meathook',':data/c_RocketSm.png',':data/mRocketSm.3dsN',0.2,1,0,0,8192+1,1,8,44, 7, 15,151,49,20, 13,131,39,18, 2500);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type, wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2, wpn1Range)
  VALUES (302,'Large Rocket BuDi Scudpox',':data/c_RocketLg.png',':data/mRocketLg.3dsN',3.8,0,0,1,1,3,12,330, 3, 12,751,249,100, 6,731,239,90, 25000);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type, wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2, wpn1Range)
  VALUES (303,'Small Missile CrFr Snowball',':data/c_MissileSm.png',':data/mMissileSm.3dsN',0.1,1,0,0,8192+1,1,8,55, 7, 10,126,24,15, 9,106,14,13, 2500);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type, wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2, wpn1Range)
  VALUES (304,'Large Missile SlPo Sideviper',':data/c_MissileLg.png',':data/mMissileLg.3dsN',3.8,0,0,1,1,3,12,350, 3, 15,701,199,90, 4,681,189,89, 25000);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,health, wpn1Mode,wpn1Range,ammo1Type,ammo1Qty, wpn2Mode,wpn2Range,ammo2Type) VALUES (305,'Laser Pistol',':data/c_LaserPistol.png',':data/mLaserPistol.3ds',0.1,1,0,0,8192+256+1,1,1,2, 80,6, 5,250,1,100, 5,250,8);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1,wpn1AP1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2)
  VALUES (306,'Heavy Arms Ammunition PiRa',':data/c_HeavyArmsAmmo.png',':data/mHeavyArmsAmmo.3ds',0.05,0,1,0,8192+1,1,5,1, 2,13,71,29,0,1, 5,61,19,10);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1,wpn1AP1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2)
  VALUES (307,'Heavy Arms Ammunition PiBl',':data/c_HeavyArmsAmmo.png',':data/mHeavyArmsAmmo.3ds',0.05,0,1,0,8192+1,1,5,1, 2,13,71,29,0,1, 8,61,19,10);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1,wpn1AP1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2)
  VALUES (308,'Heavy Arms Ammunition PiAc',':data/c_HeavyArmsAmmo.png',':data/mHeavyArmsAmmo.3ds',0.05,0,1,0,8192+1,1,5,1, 2,13,71,29,0,1, 1,61,19,10);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1,wpn1AP1)
 VALUES (309,'Stasis Beam Lens',':data/c_EnergyWeapon.png',':data/mEnergyWeaponLens.3ds',0.0,0,0,1,8192+1,2,3,3, 1,3,151,99,0,0);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1,wpn1AP1)
 VALUES (310,'Neutron Beam Lens',':data/c_EnergyWeapon.png',':data/mEnergyWeaponLens.3ds',0.0,0,0,1,8192+1,2,3,3, 1,5,151,99,0,0);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1,wpn1AP1)
 VALUES (311,'Electron Beam Lens',':data/c_EnergyWeapon.png',':data/mEnergyWeaponLens.3ds',0.0,0,0,1,8192+1,2,3,3, 1,7,151,99,0,0);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1,wpn1AP1)
 VALUES (312,'Erg Ray Lens',':data/c_EnergyWeapon.png',':data/mEnergyWeaponLens.3ds',0.0,0,0,1,8192+1,2,3,3, 1,8,151,99,0,0);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1,wpn1AP1)
 VALUES (313,'Freeze Ray Lens',':data/c_EnergyWeapon.png',':data/mEnergyWeaponLens.3ds',0.0,0,0,1,8192+1,2,3,3, 1,9,151,99,0,0);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1,wpn1AP1)
 VALUES (314,'Compressor Beam Lens',':data/c_EnergyWeapon.png',':data/mEnergyWeaponLens.3ds',0.0,0,0,1,8192+1,2,3,3, 1,10,151,99,0,0);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1,wpn1AP1)
 VALUES (315,'Disruptor Beam Lens',':data/c_EnergyWeapon.png',':data/mEnergyWeaponLens.3ds',0.0,0,0,1,8192+1,2,3,3, 1,11,151,99,0,0);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1,wpn1AP1)
 VALUES (316,'Heat Ray Lens',':data/c_EnergyWeapon.png',':data/mEnergyWeaponLens.3ds',0.0,0,0,1,8192+1,2,3,3, 1,12,151,99,0,0);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1,wpn1AP1)
 VALUES (317,'Proton Beam Lens',':data/c_EnergyWeapon.png',':data/mEnergyWeaponLens.3ds',0.0,0,0,1,8192+1,2,3,3, 1,13,151,99,0,0);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1,wpn1AP1)
 VALUES (318,'Disintegrator Ray Lens',':data/c_EnergyWeapon.png',':data/mEnergyWeaponLens.3ds',0.0,0,0,1,8192+1,2,3,3, 1,15,151,99,0,0);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond)
 VALUES (319,'Weapon Deployment Bay Module',':data/c_BayModule.png',':data/ModuleBay.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, wpn1Mode,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2) VALUES (320,'Bomb BlRa Los Alamos',':data/c_Bomb.png',':data/mBomb.3dsX',0.0,0,0,1,8192+1,2,8,250, 3,8,751,249,100, 5,151,99,250);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, wpn1Mode,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2) VALUES (321,'Bomb BlPi Porcupine',':data/c_Bomb.png',':data/mBomb.3dsX',0.0,0,0,1,8192+1,2,8,250, 3,8,751,249,100, 13,251,199,120);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, wpn1Mode,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2) VALUES (322,'Bomb ElSt Tesla',':data/c_Bomb.png',':data/mBomb.3dsX',0.0,0,0,1,8192+1,2,8,250, 3,7,751,249,200, 3,151,99,100);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, wpn1Mode,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2) VALUES (323,'Bomb SuPo Muffler',':data/c_Bomb.png',':data/mBomb.3dsX',0.0,0,0,1,8192+1,2,8,250, 3,2,651,249,100, 4,151,199,150);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, wpn1Mode,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2) VALUES (324,'Bomb SlCr Meat Slicer',':data/c_Bomb.png',':data/mBomb.3dsX',0.0,0,0,1,8192+1,2,8,250, 3,15,351,249,100, 10,251,249,100);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (325,'Seed',':data/c_Seed.png',':data/mSeed.3ds',0.0,0,0,1,8192+1,1,1,.01, 1);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (326,'Small Flower Pot',':data/c_FlowerPotSmall.png',':data/mFlowerPotSmall.3ds',0.0,0,0,1,8192+1,1,1,1, 10);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (327,'Large Flower Pot',':data/c_FlowerPotLarge.png',':data/mFlowerPotLarge.3ds',0.0,0,0,1,8192+1,2,2,10, 100);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (329,'Picture Frame 60x34cm',':data/c_PictureFrame.png',':data/mPictureFrame60x34.3ds',1.225,0,0,1,8192+1,2,1,.4, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (330,'Picture Frame 22x28cm',':data/c_PictureFrame.png',':data/mPictureFrame22x28.3ds',1.012,0,0,1,8192+1,1,1,.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (331,'Picture Frame 120x68cm',':data/c_PictureFrame.png',':data/mPictureFrame120x68.3ds',2.35,0,0,1,8192+1,4,2,.6, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, food) VALUES (332,'Head',':data/c_Skull.png','',.5,0,0,1,2+1,1,1,.8, 1, 13); -- Model is created from DNA.
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, light, wpn1Mode,wpn1Range,wpn1Effect1,wpn1Damage1,wpn1DamageMod1) VALUES (333,'Candle',':data/c_Candle.png',':data/mCandle.3ds',0,0,0,1,8192+65,1,1,1, 2, 3, 2,.5,12,1,0);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,bitsWear) VALUES (334,'Type O Preon Ring',':data/c_PreonRingO.png',':data/mPreonRing.3ds',0.1,1,0,0,8192+1,1,1,.1, 9,1089470464);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,bitsWear) VALUES (335,'Type B Preon Ring',':data/c_PreonRingB.png',':data/mPreonRing.3ds',0.1,1,0,0,8192+1,1,1,.1, 9,1089470464);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,bitsWear) VALUES (336,'Type A Preon Ring',':data/c_PreonRingA.png',':data/mPreonRing.3ds',0.1,1,0,0,8192+1,1,1,.1, 9,1089470464);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,bitsWear) VALUES (337,'Type F Preon Ring',':data/c_PreonRingF.png',':data/mPreonRing.3ds',0.1,1,0,0,8192+1,1,1,.1, 9,1089470464);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,bitsWear) VALUES (338,'Type G Preon Ring',':data/c_PreonRingG.png',':data/mPreonRing.3ds',0.1,1,0,0,8192+1,1,1,.1, 9,1089470464);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,bitsWear) VALUES (339,'Type K Preon Ring',':data/c_PreonRingK.png',':data/mPreonRing.3ds',0.1,1,0,0,8192+1,1,1,.1, 9,1089470464);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,bitsWear) VALUES (340,'Type M Preon Ring',':data/c_PreonRingM.png',':data/mPreonRing.3ds',0.1,1,0,0,8192+1,1,1,.1, 9,1089470464);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, capwid,capht) VALUES (341,'ParcX Box',':data/c_ParcX.png',':data/mParcX.3ds',0.0,0,0,1,1,3,2,.22, 1, 3,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,light) VALUES (342,'Fire',':data/c_StoneFirePit.png',':data/mStoneFirePit.3ds',0.0,0,0,1,0,12,12,100, 50,10);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (343,'Tepee',':data/c_Tepee.png',':data/mTepee.3ds',0.0,0,0,1,8192+1,3,8,50, 100);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (344,'Pressure Tent',':data/c_PressureTent.png',':data/mPressureTent.3ds',0,0,0,1,8192+1,3,8,50, 100);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, light) VALUES (345,'Beacon',':data/c_Beacon.png',':data/mBeacon.3ds',.15,0,0,1,8192+1,1,1,1, 10, 10);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (346,'Drum',':data/c_Drum.png',':data/mDrum.3ds',1.44,0,0,1,8192+1,4,5,1, 5);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, food,newcond) VALUES (347,'Ice Cream',':data/c_IceCream.png',':data/mIceCreamBar.3ds',0.0,0,0,1,20480+3,1,1,1, 20,2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight, upVectorX,upVectorY,upVectorZ, combits,sizewid,sizeht,wt, newcond,light) VALUES (349,'Burning Barrel',':data/c_BurningBarrel.png',':data/mCrudeOil.3dsN',0.0, 0,0,1, 0,12,12,10, 50,5);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (351,'Life Support Gravity Module',':data/c_LifeSupportModule.png',':data/ModuleLifeSupport.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (352,'Capacitor Module',':data/c_CapacitorModule.png',':data/ModuleCapacitor.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (353,'Spacecraft Blueprint Drawing',':data/c_ScDesignDrawing.png',':data/Blueprint.SoH',0.05,1,0,0,1,1,1,.01);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (354,'Spacecraft Blueprint Disk',':data/c_ScDesignDisk.png',':data/Disk.SoH',0.05,1,0,0,1,1,1,.22);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, capwid,capht) VALUES (355,'TEU Container',':data/c_Container.png',':data/mCrate.3dsN',0.0,0,0,1,1,40,16,5290, 50, 24,50);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (356,'Refining Hydrogen from Atmosphere Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (357,'Refining Air from Atmosphere Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (358,'Helicopter Apparatus Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (359,'Space Rocket Apparatus Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (360,'Space Fighter F1 Apparatus Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (361,'Space Transport Apparatus Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (362,'Dynamite Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (363,'Heavy Arms Ammo Pi Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (364,'Plastic Explosive Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (365,'Bomb Bl Black Cat Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (366,'Refining Fertilizer from Atmosphere Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (367,'Refining Fertilizer from Air Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (369,'ParcX Uniform',':data/c_ParcXUniform.png',':data/mLinenShirt.3dsN',0.0,0,0,1,8192+1,2,2,.5, 0, 3,14592);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (370,'Large Rocket Motor Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (371,'Preon Ring Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (372,'Capris and Halter',':data/c_CaprisHalter.png',':data/mLinenShirt.3dsN',0.0,0,0,1,8192+1,2,2,.5, 0, 3,14592);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, food) VALUES (373,'Brain',':data/c_Brain.png',':data/Brain.SoH',.25,0,0,1,4096+3,1,1,.5, 1, 8);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (374,'Pressure Tent Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (375,'Beacon Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (376,'Ice Cream Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (377,'Life Support Gravity Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (378,'Capacitor Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (380,'Candy Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (381,'Hydrogen from Natural Gas Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, food) VALUES (382,'Gills',':data/c_Gills.png',':data/Gills.SoH',.25,0,0,1,4096+3,1,1,.5, 1, 6);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (383,'Computer Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (384,'Motor Bike Apparatus Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, food) VALUES (385,'Heart',':data/c_Heart.png',':data/Heart.SoH',.25,0,0,1,4096+3,1,1,.5, 1, 9);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (386,'Rifle Ammunition Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (387,'Distilling Whiskey Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (389,'Pistol Ammunition Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (390,'Gun Powder Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (391,'Generating Electricity using Coal Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (392,'Generating Electricity using Gasoline Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (393,'Generating Electricity using Hydrogen Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (394,'Generating Electricity using Natural Gas Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (395,'Generating Electricity using Sunlight Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (396,'Generating Electricity using Wind Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (397,'Generating Electricity using Radioactives Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (398,'Electronic Part Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (399,'Farm Machinery Apparatus Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (400,'Knife using Metal and Plastic Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (401,'Water using Air and Hydrogen Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (402,'Pistol Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (403,'Distilling Vodka Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, food) VALUES (404,'Lungs',':data/c_Lungs.png',':data/Lungs.SoH',.25,0,0,1,4096+3,1,1,.5, 1, 6);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, food) VALUES (405,'Stomach',':data/c_Stomach.png',':data/Stomach.SoH',.25,0,0,1,4096+3,1,1,.5, 1, 7);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (406,'Rifle Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (407,'Кала́шников Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (408,'Grey BDU',':data/c_GreyBDU.png',':data/mLinenShirt.3dsN',0.0,0,0,1,8192+1,2,2,.5, 0, 3,14592);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (409,'Khaki BDU',':data/c_KhakiBDU.png',':data/mLinenShirt.3dsN',0.0,0,0,1,8192+1,2,2,.5, 0, 3,14592);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (410,'Olive Drab BDU',':data/c_GreenBDU.png',':data/mLinenShirt.3dsN',0.0,0,0,1,8192+1,2,2,.5, 0, 3,14592);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (411,'Refining Gasoline Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, capwid,capht) VALUES (412,'Red Binder',':data/c_RedBinder.png',':data/RedBinder.SoH',0.0,1,0,0,8192+1,1,1,1.1, 2, 1,100);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (413,'Generating Electricity using Water in Environment Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (414,'Nitrate Explosive Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (416,'Refining Petrochemicals Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (417,'Refining Hydrogen from Water Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (418,'Grav Coupling Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (419,'Cryo Heat Sink Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (420,'Magmium Knife Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, capwid,capht) VALUES (421,'Green Binder',':data/c_GreenBinder.png',':data/GreenBinder.SoH',0.0,1,0,0,8192+1,1,1,1.1, 2, 1,100);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond, capwid,capht) VALUES (422,'Blue Binder',':data/c_BlueBinder.png',':data/BlueBinder.SoH',0.0,1,0,0,8192+1,1,1,1.1, 2, 1,100);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (423,'Io Tube Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (424,'Plastic Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (425,'Mine Machinery Apparatus Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (426,'Small Missile Bl Zipper Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (429,'Large Missile Bl Thumper Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (430,'Pastry Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (431,'Refining Air from Water Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (432,'Pharmaceuticals using Petrochemicals Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (434,'Metal EV Suit Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (435,'Mining Radioactives Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (436,'Small Rocket Bl Partypopper Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (439,'Large Rocket Bl Thud Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (440,'Small Rocket Motor Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (442,'Plastic Sailboat Apparatus Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (443,'Tank Apparatus Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (444,'APC Apparatus Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (445,'Metal EV Helmet Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (446,'Magmium EV Helmet Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (447,'Commercial Ship Apparatus Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (448,'Battle Ship Apparatus Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (449,'Submarine Apparatus Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (450,'Weaving Textiles using Plastic Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (452,'EV Suit Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (453,'EV Helmet Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (454,'SUV Apparatus Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (455,'Distilling Gin Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (456,'Heavy Weapon Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (457,'Distilling Rum Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (459,'Distilling Tequila Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (461,'Lantern Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (462,'Mining Lumenite Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (463,'Blank Disk Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (465,'Antidote using Herbs Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (466,'Antidote using Minerals Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (467,'Antidote using Petrochemicals Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (468,'Antitoxin using Animal Carcass Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (469,'Antitoxin using Eggs Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (470,'Shotgun Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (472,'Brown Uniform',':data/c_BrownUniform.png',':data/mLinenShirt.3dsN',0.0,0,0,1,8192+1,2,2,.5, 0, 3,14592);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (473,'Mining Eludium Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (474,'Shotgun Ammunition Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (476,'Glue Gun Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (477,'Grey Uniform',':data/c_GreyUniform.png',':data/mLinenShirt.3dsN',0.0,0,0,1,8192+1,2,2,.5, 0, 3,14592);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (478,'Soldering Iron Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (479,'Red Uniform',':data/c_RedUniform.png',':data/mLinenShirt.3dsN',0.0,0,0,1,8192+1,2,2,.5, 0, 3,14592);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (480,'Green Uniform',':data/c_GreenUniform.png',':data/mLinenShirt.3dsN',0.0,0,0,1,8192+1,2,2,.5, 0, 3,14592);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (481,'Blue Uniform',':data/c_BlueUniform.png',':data/mLinenShirt.3dsN',0.0,0,0,1,8192+1,2,2,.5, 0, 3,14592);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (482,'Teal Uniform',':data/c_TealUniform.png',':data/mLinenShirt.3dsN',0.0,0,0,1,8192+1,2,2,.5, 0, 3,14592);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (483,'Gold Uniform',':data/c_GoldUniform.png',':data/mLinenShirt.3dsN',0.0,0,0,1,8192+1,2,2,.5, 0, 3,14592);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (484,'Pink Uniform',':data/c_PinkUniform.png',':data/mLinenShirt.3dsN',0.0,0,0,1,8192+1,2,2,.5, 0, 3,14592);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (485,'Grav Pack Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (486,'White Uniform',':data/c_WhiteUniform.png',':data/mLinenShirt.3dsN',0.0,0,0,1,8192+1,2,2,.5, 0, 3,14592);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (488,'Black Uniform',':data/c_BlackUniform.png',':data/mLinenShirt.3dsN',0.0,0,0,1,8192+1,2,2,.5, 0, 3,14592);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (489 ,'Mining Vulcanite Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (490,'Mining Bolite Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (491,'Refining Ioplasma Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (492,'Refining Cryozine Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (493,'Extracting Magmex Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (494,'Extracting Myrathane Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (495,'Drilling Phlogiston Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (496,'Drilling Polytaride Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (497,'Smelting Vulcium Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (498,'Smelting Magmium Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (499,'Vulcium Knife Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, armorSpec, newcond,bitsWear) VALUES (500,'Orange Uniform',':data/c_OrangeUniform.png',':data/mLinenShirt.3dsN',0.0,0,0,1,8192+1,2,2,.5, 0, 3,14592);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (502,'Magmium EV Suit Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (503,'Vulcium EV Suit Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (504,'Vulcium EV Helmet Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (506,'Oscilloscope Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (507,'Space Fighter F2 Apparatus Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (508,'Command And Control Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (509,'Gravity Drive Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (510,'Rocket Drive Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (512,'Life Support Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (513,'Hydrogen Power Plant Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (514,'Energy Shield Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (515,'Kinetic Shield Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (516,'Cloaking Shield Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (517,'Surgery Medical Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (518,'Genetics Medical Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (519,'Cybernetics Medical Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (520,'Personnel Transporter Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (521,'Vehicle Transporter Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (522,'Energy Weapon Turret Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (523,'Weapon Launcher Turret Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (524,'Energy Weapon Bay Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (525,'Weapon Launcher Bay Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (526,'Atmosphere Condenser Bay Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (527,'Prospector Bay Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (528,'Tractor Beam Bay Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (529,'Wormhole P FTL Drive Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (530,'Warp 1 FTL Drive Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (531,'Ballistic Weapon Bay Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (532,'Ballistic Weapon Turret Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (533,'Sensor Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (534,'Magmium Heat Sink Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (535,'Vulcium Heat Sink Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (536,'Myrasplicer Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (537,'Microscope Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (538,'Gigacell Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (539,'Tesloid Dissipator Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (540,'Nanopath Calibrator Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (543,'Particle Collector Bay Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (544,'DNA Sample Kit Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (547,'Wildcatter Bay Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (548,'Siphon Bay Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (549,'Laser Rifle Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (550,'Small Rocket BlBu Firecracker Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (551,'Large Rocket BlBu Molotov Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (552,'Small Missile BlBu Barnburner Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (553,'Large Missile BlBu Fireball Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (554,'Small Rocket SlPi Meathook Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (555,'Large Rocket BuDi Scudpox Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (556,'Small Missile CrFr Snowball Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (557,'Large Missile SlPo Sideviper Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (558,'Laser Pistol Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (559,'Heavy Arms Ammo PiRa Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (560,'Heavy Arms Ammo PiBl Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (561,'Heavy Arms Ammo PiAc Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (562,'Stasis Beam Lens Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (563,'Neutron Beam Lens Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (564,'Electron Beam Lens Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (565,'Erg Ray Lens Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (566,'Freeze Ray Lens Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (567,'Compressor Beam Lens Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (568,'Disruptor Beam Lens Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (569,'Heat Ray Lens Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (570,'Proton Beam Lens Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (571,'Disintegrator Ray Lens Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (572,'Weapon Deployment Bay Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (573,'Bomb BlRa Los Alamos Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (574,'Bomb BlPi Porcupine Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (575,'Bomb ElSt Tesla Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (576,'Bomb SuPo Muffler Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (577,'Bomb SlCr Meat Slicer Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (584,'Candle using Petrochemicals Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (589,'Wormhole PN FTL Drive Module',':data/c_FTLModule.png',':data/ModuleFTL.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (590,'Wormhole PNN FTL Drive Module',':data/c_FTLModule.png',':data/ModuleFTL.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (591,'Warp 2 FTL Drive Module',':data/c_FTLModule.png',':data/ModuleFTL.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (592,'Warp 3 FTL Drive Module',':data/c_FTLModule.png',':data/ModuleFTL.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (593,'Warp 4 FTL Drive Module',':data/c_FTLModule.png',':data/ModuleFTL.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (594,'Warp 5 FTL Drive Module',':data/c_FTLModule.png',':data/ModuleFTL.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (595,'Warp 6 FTL Drive Module',':data/c_FTLModule.png',':data/ModuleFTL.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (596,'Warp 7 FTL Drive Module',':data/c_FTLModule.png',':data/ModuleFTL.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (597,'Warp 8 FTL Drive Module',':data/c_FTLModule.png',':data/ModuleFTL.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (598,'Warp 9 FTL Drive Module',':data/c_FTLModule.png',':data/ModuleFTL.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (599,'Wormhole PN FTL Drive Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (600,'Wormhole PNN FTL Drive Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (601,'Warp 2 FTL Drive Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (602,'Warp 3 FTL Drive Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (603,'Warp 4 FTL Drive Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (604,'Warp 5 FTL Drive Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (605,'Warp 6 FTL Drive Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (606,'Warp 7 FTL Drive Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (607,'Warp 8 FTL Drive Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (608,'Warp 9 FTL Drive Module Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (609,'Adamantite',':data/c_Adamantite.png',':data/Adamantite.SoH',0.1,0,0,1,1,2,2,8.5);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (610,'Adamantine',':data/c_Adamantine.png',':data/Adamantine.SoH',0.1,1,0,0,8192+1,2,1,5);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, ammo1Type,wpn1Effect1,wpn1Damage1,wpn1DamageMod1,wpn1Radius1, wpn1Effect2,wpn1Damage2,wpn1DamageMod2,wpn1Radius2) VALUES (611,'Viathol',':data/c_Viathol.png',':data/mCrudeOil.3dsN',0.0,0,0,1,1,3,3,9, 9,8,21,19,10, 1,21,19,15);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (612,'Flomentum',':data/c_Flomentum.png',':data/mPetrochemicals.3dsN',0.0,0,0,1,1,3,3,9);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (613 ,'Mining Adamantite Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (614 ,'Smelting Adamantine Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (615 ,'Drilling Viathol Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (616 ,'Extracting Flomentum Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (617,'Storage Crystal',':data/c_StorageCrystal.png',':data/StorageCrystal.SoH',0.05,1,0,0,8192+1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (618,'Storage Crystal Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (619,'Thumb Drive',':data/c_ThumbDrive.png',':data/ThumbDrive.SoH',0.05,1,0,0,8192+1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (620,'Thumb Drive Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (621,'Jewelry Storage Device Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,bitsWear) VALUES (622,'Thumb Drive Bracelet',':data/c_MetalJewelry.png',':data/mMetalJewelry.3dsN',0.05,0,1,0,8192+1,1,1,.3, 2,1879048192);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,bitsWear) VALUES (623,'Storage Crystal Bracelet',':data/c_MetalJewelry.png',':data/mMetalJewelry.3dsN',0.05,0,1,0,8192+1,1,1,.3, 2,1879048192);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,bitsWear) VALUES (624,'Thumb Drive Necklace',':data/c_CrystalJewelry.png',':data/mCrystalJewelry.3dsN',0.1,0,1,0,8192+1,1,1,.3, 1,524288);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,bitsWear) VALUES (625,'Storage Crystal Necklace',':data/c_CrystalJewelry.png',':data/mCrystalJewelry.3dsN',0.1,0,1,0,8192+1,1,1,.3, 1,524288);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,bitsWear) VALUES (626,'Thumb Drive Ring',':data/c_GemJewelry.png',':data/mGemJewelry.3dsN',0.1,0,1,0,8192+1,1,1,.1, 2,1089470464);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond,bitsWear) VALUES (627,'Storage Crystal Ring',':data/c_GemJewelry.png',':data/mGemJewelry.3dsN',0.1,0,1,0,8192+1,1,1,.1, 2,1089470464);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (628,'Coal Power Plant Module',':data/c_PowerPlantModule.png',':data/ModulePower.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (629,'Gasoline Power Plant Module',':data/c_PowerPlantModule.png',':data/ModulePower.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (630,'Hydroelectric Power Plant Module',':data/c_PowerPlantModule.png',':data/ModulePower.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (631,'Natural Gas Power Plant Module',':data/c_PowerPlantModule.png',':data/ModulePower.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (632,'Nuclear Power Plant Module',':data/c_PowerPlantModule.png',':data/ModulePower.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (633,'Solar Power Plant Module',':data/c_PowerPlantModule.png',':data/ModulePower.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (634,'Wind Power Plant Module',':data/c_PowerPlantModule.png',':data/ModulePower.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (635,'Wood Hull',':data/c_Wood.png',':data/ModuleHull.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (636,'Lumber Hull',':data/c_Lumber.png',':data/ModuleHull.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (637,'Stone Hull',':data/c_Stone.png',':data/ModuleHull.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, newcond) VALUES (638,'Adamantine Hull',':data/c_Adamantine.png',':data/ModuleHull.SoH',0.0,0,0,1,8192+1,1,2,2.2, 2);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (650,'Building Blueprint Drawing',':data/c_BdDesignDrawing.png',':data/Blueprint.SoH',0.05,1,0,0,1,1,1,.01);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (651,'Building Blueprint Disk',':data/c_BdDesignDisk.png',':data/Disk.SoH',0.05,1,0,0,1,1,1,.22);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt, health) VALUES (652,'Nuclear Fuel Rod',':data/c_FuelRod.png',':data/FuelRod.SoH',0.2,-1,0,0,8192+257,1,8,100, 9);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (653,'Nuclear Fuel Rod Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (655,'Plutonium',':data/c_Plutonium.png',':data/Plutonium.SoH',0.5,0,0,1,8192+1,2,2,20);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (656,'Generating Electricity using Nuclear Fuel Rod Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (657,'Plutonium Patent',':data/c_Patent.png',':data/Patent.SoH',0.05,1,0,0,1,1,1,.05);
INSERT INTO b_Commodity (id,name,iconPath,modelPath,insertHeight,upVectorX,upVectorY,upVectorZ,combits,sizewid,sizeht,wt) VALUES (663,'Spacecraft Title',':data/c_Certificate.png',':data/Certificate.SoH',0.05,1,0,0,1,1,1,.05);
-- End of b_Commodity

-- Armor values are updated separately so I can compare/tune them easily.
-- Wearable Personal Armor
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Leather Armor
  armorEffectAbsorbBludgeon=1,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=1,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=2,
  armorModeAbsorbContact=2,
  armorModeAbsorbProjectile=2,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=2,
  armorModeBlockContact=2,
  armorModeBlockProjectile=2,
  armorModeBlockPsionic=0,
  armorSpec=8 WHERE id=17;                                      -- Leather Armor
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Jeans and Shirt
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=1,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=1,
  armorModeAbsorbContact=0,
  armorModeAbsorbProjectile=0,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=1,
  armorModeBlockContact=1,
  armorModeBlockProjectile=1,
  armorModeBlockPsionic=0,
  armorSpec=0 WHERE id=21 or id=369 or id=372 or id=408 or id=409 or id=410 or id=472 or id=477 or id=479 or id=480 or id=481 or id=482 or id=483 or id=484 or id=486 or id=488;   -- Jeans and Shirt, Uniforms, Capris, BDUs
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Flak Helmet
  armorEffectAbsorbBludgeon=1,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=1,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=2,
  armorModeAbsorbContact=2,
  armorModeAbsorbProjectile=2,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=2,
  armorModeBlockContact=2,
  armorModeBlockProjectile=2,
  armorModeBlockPsionic=0,
  armorSpec=13 WHERE id=41;                                     -- Flak Helmet
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Metal EV Suit
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=1,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=1,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=1,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=1,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=1,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=1,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=10,
  armorModeAbsorbContact=10,
  armorModeAbsorbProjectile=10,
  armorModeAbsorbPsionic=2,
  armorModeBlockArea=10,
  armorModeBlockContact=10,
  armorModeBlockProjectile=10,
  armorModeBlockPsionic=2,
  armorSpec=7 WHERE id=65;                                      -- Metal EV Suit
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- EV Helmet
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=1,
  armorEffectAbsorbCrush=1,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=1,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=1,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=1,
  armorEffectBlockCrush=1,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=1,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=2,
  armorModeAbsorbContact=2,
  armorModeAbsorbProjectile=2,
  armorModeAbsorbPsionic=1,
  armorModeBlockArea=1,
  armorModeBlockContact=1,
  armorModeBlockProjectile=1,
  armorModeBlockPsionic=0,
  armorSpec=0 WHERE id=72;                                      -- EV Helmet
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- EV Suit
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=2,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=2,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=2,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=2,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=3,
  armorModeAbsorbContact=3,
  armorModeAbsorbProjectile=3,
  armorModeAbsorbPsionic=1,
  armorModeBlockArea=3,
  armorModeBlockContact=3,
  armorModeBlockProjectile=3,
  armorModeBlockPsionic=1,
  armorSpec=0 WHERE id=80;                                      -- EV Suit
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Metal Armor
  armorEffectAbsorbBludgeon=1,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=1,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=1,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=1,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=1,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=8,
  armorModeAbsorbContact=8,
  armorModeAbsorbProjectile=8,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=10,
  armorModeBlockContact=10,
  armorModeBlockProjectile=10,
  armorModeBlockPsionic=0,
  armorSpec=15 WHERE id=90;                                     -- Metal Armor
UPDATE b_Commodity SET armorEffectAbsorbAcid=1,               -- Plastic Armor
  armorEffectAbsorbBludgeon=1,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=1,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=1,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=1,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=1,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=2,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=1,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=6,
  armorModeAbsorbContact=6,
  armorModeAbsorbProjectile=6,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=7,
  armorModeBlockContact=7,
  armorModeBlockProjectile=7,
  armorModeBlockPsionic=0,
  armorSpec=1 WHERE id=91;                                      -- Plastic Armor
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Flak Vest
  armorEffectAbsorbBludgeon=1,
  armorEffectAbsorbCold=1,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=1,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=1,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=1,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=1,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=4,
  armorModeAbsorbContact=4,
  armorModeAbsorbProjectile=4,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=5,
  armorModeBlockContact=5,
  armorModeBlockProjectile=5,
  armorModeBlockPsionic=0,
  armorSpec=13 WHERE id=92;                                     -- Flak Vest
UPDATE b_Commodity SET armorEffectAbsorbAcid=1,                 -- Leather Clothing
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=1,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=1,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=1,
  armorModeAbsorbContact=1,
  armorModeAbsorbProjectile=1,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=1,
  armorModeBlockContact=1,
  armorModeBlockProjectile=1,
  armorModeBlockPsionic=0,
  armorSpec=8 WHERE id=95;                                      -- Leather Clothing
UPDATE b_Commodity SET armorEffectAbsorbAcid=1,               -- Leather Helmet
  armorEffectAbsorbBludgeon=1,
  armorEffectAbsorbCold=1,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=1,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=1,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=1,
  armorModeAbsorbContact=1,
  armorModeAbsorbProjectile=1,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=1,
  armorModeBlockContact=1,
  armorModeBlockProjectile=1,
  armorModeBlockPsionic=0,
  armorSpec=8 WHERE id=96;                                      -- Leather Helmet
UPDATE b_Commodity SET armorEffectAbsorbAcid=1,               -- Plastic Helmet
  armorEffectAbsorbBludgeon=1,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=1,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=1,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=1,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=1,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=1,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=1,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=1,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=3,
  armorModeAbsorbContact=3,
  armorModeAbsorbProjectile=3,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=3,
  armorModeBlockContact=3,
  armorModeBlockProjectile=3,
  armorModeBlockPsionic=0,
  armorSpec=1 WHERE id=97;                                      -- Plastic Helmet
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Metal Helmet
  armorEffectAbsorbBludgeon=1,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=1,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=1,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=1,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=1,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=1,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=1,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=1,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=4,
  armorModeAbsorbContact=4,
  armorModeAbsorbProjectile=4,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=5,
  armorModeBlockContact=5,
  armorModeBlockProjectile=5,
  armorModeBlockPsionic=0,
  armorSpec=15 WHERE id=105;                                    -- Metal Helmet
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Metal EV Helmet
  armorEffectAbsorbBludgeon=1,
  armorEffectAbsorbCold=1,
  armorEffectAbsorbCrush=1,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=1,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=1,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=1,
  armorEffectBlockCold=1,
  armorEffectBlockCrush=1,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=1,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=5,
  armorModeAbsorbContact=5,
  armorModeAbsorbProjectile=5,
  armorModeAbsorbPsionic=1,
  armorModeBlockArea=5,
  armorModeBlockContact=5,
  armorModeBlockProjectile=5,
  armorModeBlockPsionic=1,
  armorSpec=7 WHERE id=106;                                     -- Metal EV Helmet
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Magmium EV Suit
  armorEffectAbsorbBludgeon=1,
  armorEffectAbsorbCold=1,
  armorEffectAbsorbCrush=1,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=1,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=1,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=1,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=1,
  armorEffectBlockCold=1,
  armorEffectBlockCrush=1,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=1,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=1,
  armorEffectBlockSlash=1,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=15,
  armorModeAbsorbContact=15,
  armorModeAbsorbProjectile=15,
  armorModeAbsorbPsionic=3,
  armorModeBlockArea=15,
  armorModeBlockContact=15,
  armorModeBlockProjectile=15,
  armorModeBlockPsionic=4,
  armorSpec=12 WHERE id=153;                                    -- Magmium EV Suit
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Magmium EV Helmet
  armorEffectAbsorbBludgeon=1,
  armorEffectAbsorbCold=1,
  armorEffectAbsorbCrush=1,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=1,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=1,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=1,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=1,
  armorEffectBlockCrush=1,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=1,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=1,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=7,
  armorModeAbsorbContact=7,
  armorModeAbsorbProjectile=7,
  armorModeAbsorbPsionic=2,
  armorModeBlockArea=7,
  armorModeBlockContact=7,
  armorModeBlockProjectile=7,
  armorModeBlockPsionic=2,
  armorSpec=12 WHERE id=154;                                    -- Magmium EV Helmet
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Vulcium EV Suit
  armorEffectAbsorbBludgeon=1,
  armorEffectAbsorbCold=1,
  armorEffectAbsorbCrush=1,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=1,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=1,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=1,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=1,
  armorEffectBlockCold=1,
  armorEffectBlockCrush=1,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=1,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=1,
  armorEffectBlockSlash=1,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=30,
  armorModeAbsorbContact=30,
  armorModeAbsorbProjectile=30,
  armorModeAbsorbPsionic=4,
  armorModeBlockArea=30,
  armorModeBlockContact=30,
  armorModeBlockProjectile=30,
  armorModeBlockPsionic=5,
  armorSpec=5 WHERE id=155;                                     -- Vulcium EV Suit
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Vulcium EV Helmet
  armorEffectAbsorbBludgeon=1,
  armorEffectAbsorbCold=1,
  armorEffectAbsorbCrush=1,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=1,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=1,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=1,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=1,
  armorEffectBlockCold=1,
  armorEffectBlockCrush=1,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=1,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=1,
  armorEffectBlockSlash=1,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=10,
  armorModeAbsorbContact=10,
  armorModeAbsorbProjectile=10,
  armorModeAbsorbPsionic=2,
  armorModeBlockArea=10,
  armorModeBlockContact=10,
  armorModeBlockProjectile=10,
  armorModeBlockPsionic=3,
  armorSpec=5 WHERE id=156;                                     -- Vulcium EV Helmet
-- Vehicle Armor
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Space Fighter F2
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=15,
  armorModeAbsorbContact=15,
  armorModeAbsorbProjectile=15,
  armorModeAbsorbPsionic=2,
  armorModeBlockArea=15,
  armorModeBlockContact=15,
  armorModeBlockProjectile=15,
  armorModeBlockPsionic=2
  WHERE id=231;                                                 -- Space Fighter F2
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- APC
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=30,
  armorModeAbsorbContact=30,
  armorModeAbsorbProjectile=30,
  armorModeAbsorbPsionic=5,
  armorModeBlockArea=30,
  armorModeBlockContact=30,
  armorModeBlockProjectile=30,
  armorModeBlockPsionic=5
  WHERE id=104;                                                 -- APC
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Motor Bike
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=5,
  armorModeAbsorbContact=5,
  armorModeAbsorbProjectile=5,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=5,
  armorModeBlockContact=5,
  armorModeBlockProjectile=5,
  armorModeBlockPsionic=0
  WHERE id=133;                                                 -- Motor Bike
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Farm Machinery
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=12,
  armorModeAbsorbContact=12,
  armorModeAbsorbProjectile=12,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=12,
  armorModeBlockContact=12,
  armorModeBlockProjectile=12,
  armorModeBlockPsionic=0
  WHERE id=28;                                                  -- Farm Machinery
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Helicopter
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=3,
  armorModeAbsorbContact=3,
  armorModeAbsorbProjectile=3,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=4,
  armorModeBlockContact=4,
  armorModeBlockProjectile=4,
  armorModeBlockPsionic=0
  WHERE id=6;                                                   -- Helicopter
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Sailboat
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=2,
  armorModeAbsorbContact=2,
  armorModeAbsorbProjectile=2,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=3,
  armorModeBlockContact=3,
  armorModeBlockProjectile=3,
  armorModeBlockPsionic=0
  WHERE id=132;                                                 -- Sailboat
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Space Fighter F1
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=15,
  armorModeAbsorbContact=15,
  armorModeAbsorbProjectile=15,
  armorModeAbsorbPsionic=2,
  armorModeBlockArea=15,
  armorModeBlockContact=15,
  armorModeBlockProjectile=15,
  armorModeBlockPsionic=2
  WHERE id=8;                                                   -- Space Fighter F1
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Space Rocket
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=6,
  armorModeAbsorbContact=6,
  armorModeAbsorbProjectile=6,
  armorModeAbsorbPsionic=1,
  armorModeBlockArea=6,
  armorModeBlockContact=6,
  armorModeBlockProjectile=6,
  armorModeBlockPsionic=1
  WHERE id=7;                                                   -- Space Rocket
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Space Transport
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=15,
  armorModeAbsorbContact=15,
  armorModeAbsorbProjectile=15,
  armorModeAbsorbPsionic=2,
  armorModeBlockArea=15,
  armorModeBlockContact=15,
  armorModeBlockProjectile=15,
  armorModeBlockPsionic=2
  WHERE id=9;                                                   -- Space Transport
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Submarine
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=10,
  armorModeAbsorbContact=10,
  armorModeAbsorbProjectile=10,
  armorModeAbsorbPsionic=1,
  armorModeBlockArea=10,
  armorModeBlockContact=10,
  armorModeBlockProjectile=10,
  armorModeBlockPsionic=1
  WHERE id=77;                                                  -- Submarine
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- SUV
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=7,
  armorModeAbsorbContact=7,
  armorModeAbsorbProjectile=7,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=8,
  armorModeBlockContact=8,
  armorModeBlockProjectile=8,
  armorModeBlockPsionic=0
  WHERE id=82;                                                  -- SUV
-- Building Armor (Military buildings multiply the block values by 10x)
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Wood
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=15,
  armorModeAbsorbContact=15,
  armorModeAbsorbProjectile=15,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=15,
  armorModeBlockContact=15,
  armorModeBlockProjectile=15,
  armorModeBlockPsionic=0
  WHERE id=86;                                                  -- Wood
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Lumber
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=10,
  armorModeAbsorbContact=10,
  armorModeAbsorbProjectile=10,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=10,
  armorModeBlockContact=10,
  armorModeBlockProjectile=10,
  armorModeBlockPsionic=0
  WHERE id=48;                                                  -- Lumber
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Stone
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=20,
  armorModeAbsorbContact=20,
  armorModeAbsorbProjectile=20,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=20,
  armorModeBlockContact=20,
  armorModeBlockProjectile=20,
  armorModeBlockPsionic=0
  WHERE id=76;                                                  -- Stone
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Metal
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=25,
  armorModeAbsorbContact=25,
  armorModeAbsorbProjectile=25,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=25,
  armorModeBlockContact=25,
  armorModeBlockProjectile=25,
  armorModeBlockPsionic=0
  WHERE id=51;                                                  -- Metal
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Magmium
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=30,
  armorModeAbsorbContact=30,
  armorModeAbsorbProjectile=30,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=30,
  armorModeBlockContact=30,
  armorModeBlockProjectile=30,
  armorModeBlockPsionic=0
  WHERE id=146;                                                 -- Magmium
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Vulcium
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=0,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=0,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=0,
  armorEffectAbsorbSlash=0,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=0,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=0,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=35,
  armorModeAbsorbContact=35,
  armorModeAbsorbProjectile=35,
  armorModeAbsorbPsionic=0,
  armorModeBlockArea=35,
  armorModeBlockContact=35,
  armorModeBlockProjectile=35,
  armorModeBlockPsionic=0
  WHERE id=145;                                                 -- Vulcium
-- Hull Armor by Module
-- Block values for hull modules are multiplied by the armor level (1-11 not 0-10).
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Metal Hull
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=1,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=1,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=1,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=1,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=1,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=1,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=10,
  armorModeAbsorbContact=10,
  armorModeAbsorbProjectile=10,
  armorModeAbsorbPsionic=2,
  armorModeBlockArea=10,
  armorModeBlockContact=10,
  armorModeBlockProjectile=10,
  armorModeBlockPsionic=2
  WHERE id=237;                                                 -- Metal Hull
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Magmium Hull
  armorEffectAbsorbBludgeon=1,
  armorEffectAbsorbCold=1,
  armorEffectAbsorbCrush=1,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=1,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=1,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=1,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=1,
  armorEffectBlockCold=1,
  armorEffectBlockCrush=1,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=1,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=1,
  armorEffectBlockSlash=1,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=15,
  armorModeAbsorbContact=15,
  armorModeAbsorbProjectile=15,
  armorModeAbsorbPsionic=3,
  armorModeBlockArea=15,
  armorModeBlockContact=15,
  armorModeBlockProjectile=15,
  armorModeBlockPsionic=3
  WHERE id=280;                                                 -- Magmium Hull
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Vulcium Hull
  armorEffectAbsorbBludgeon=1,
  armorEffectAbsorbCold=1,
  armorEffectAbsorbCrush=1,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=1,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=1,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=1,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=1,
  armorEffectBlockCold=1,
  armorEffectBlockCrush=1,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=1,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=1,
  armorEffectBlockSlash=1,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=25,
  armorModeAbsorbContact=25,
  armorModeAbsorbProjectile=25,
  armorModeAbsorbPsionic=4,
  armorModeBlockArea=25,
  armorModeBlockContact=25,
  armorModeBlockProjectile=25,
  armorModeBlockPsionic=4
  WHERE id=281;                                                 -- Vulcium Hull
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Adamantine Hull
  armorEffectAbsorbBludgeon=1,
  armorEffectAbsorbCold=1,
  armorEffectAbsorbCrush=1,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=1,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=1,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=1,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=1,
  armorEffectBlockCold=1,
  armorEffectBlockCrush=1,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=1,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=0,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=1,
  armorEffectBlockSlash=1,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=30,
  armorModeAbsorbContact=30,
  armorModeAbsorbProjectile=30,
  armorModeAbsorbPsionic=5,
  armorModeBlockArea=30,
  armorModeBlockContact=30,
  armorModeBlockProjectile=30,
  armorModeBlockPsionic=5
  WHERE id=638;                                                 -- Adamantine Hull
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Wood Hull
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=1,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=1,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=1,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=1,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=1,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=1,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=6,
  armorModeAbsorbContact=6,
  armorModeAbsorbProjectile=6,
  armorModeAbsorbPsionic=1,
  armorModeBlockArea=6,
  armorModeBlockContact=6,
  armorModeBlockProjectile=6,
  armorModeBlockPsionic=1
  WHERE id=635;                                                 -- Wood Hull
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Lumber Hull
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=1,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=1,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=1,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=1,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=1,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=1,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=4,
  armorModeAbsorbContact=4,
  armorModeAbsorbProjectile=4,
  armorModeAbsorbPsionic=1,
  armorModeBlockArea=4,
  armorModeBlockContact=4,
  armorModeBlockProjectile=4,
  armorModeBlockPsionic=1
  WHERE id=636;                                                 -- Lumber Hull
UPDATE b_Commodity SET armorEffectAbsorbAcid=0,                 -- Stone Hull
  armorEffectAbsorbBludgeon=0,
  armorEffectAbsorbCold=0,
  armorEffectAbsorbCrush=1,
  armorEffectAbsorbDisease=0,
  armorEffectAbsorbElectric=0,
  armorEffectAbsorbHeat=0,
  armorEffectAbsorbImplant=0,
  armorEffectAbsorbPierce=1,
  armorEffectAbsorbPoison=0,
  armorEffectAbsorbRadiation=1,
  armorEffectAbsorbSlash=1,
  armorEffectAbsorbSonic=0,
  armorEffectAbsorbStun=0,
  armorEffectAbsorbSuffocate=0,
  armorEffectBlockAcid=0,
  armorEffectBlockBludgeon=0,
  armorEffectBlockCold=0,
  armorEffectBlockCrush=1,
  armorEffectBlockDisease=0,
  armorEffectBlockElectric=0,
  armorEffectBlockHeat=0,
  armorEffectBlockImplant=0,
  armorEffectBlockPierce=1,
  armorEffectBlockPoison=0,
  armorEffectBlockRadiation=1,
  armorEffectBlockSlash=0,
  armorEffectBlockSonic=0,
  armorEffectBlockStun=0,
  armorEffectBlockSuffocate=0,
  armorModeAbsorbArea=20,
  armorModeAbsorbContact=20,
  armorModeAbsorbProjectile=20,
  armorModeAbsorbPsionic=4,
  armorModeBlockArea=20,
  armorModeBlockContact=20,
  armorModeBlockProjectile=20,
  armorModeBlockPsionic=4
  WHERE id=637;                                                 -- Stone Hull
SELECT * FROM b_Commodity;

-- Industries are represented by buildings in the universe.
-- Industries consume commodities to create other commodities.
-- Industries increase/decrease the population.
-- Buildings add to population by the industries they represent and sometimes
-- for other reasons. e.g. An office building might also employ a custodian and
-- a manager.
-- class DbIndustry
DROP TABLE b_Industry;
CREATE TABLE b_Industry
(
 id                                     integer primary key unique,
 name                                   text,                   -- Name of this industry
 pop                                    smallint DEFAULT 0,     -- Population increase per level
 emp                                    smallint DEFAULT 0,     -- Employee overhead required per building.
                                                                -- Also adds a job per level when emp > 0.
                                                                -- Adds abs(emp) jobs when emp < 0.
                                                                -- Troops = abs(emp) for military buildings.
 storage                                smallint DEFAULT 0,     -- City storage space/10 added per level.
 capacitor                              smallint DEFAULT 0,     -- City capacitor storage/10 added per level.
 -- Requirments for a Bd building design to be used for this industry.
 buildRule                              smallint DEFAULT 0,     -- 0=civilian, 1=HQ, 2=military, 3=capitol, 4=police, 5=city
 structure                              smallint DEFAULT 3,     -- BUILDING_STRUCTURE 3=Building, 0=invalid for bd, old only
 arena                                  smallint DEFAULT 0,     -- Count required, 0 = optional, -1 = not allowed.
 auditorium                             smallint DEFAULT 0,     -- Count required, 0 = optional, -1 = not allowed.
 battery                                smallint DEFAULT 0,     -- Cubic meters required, 0 = optional, -1 = not allowed.
 cargoStorage                           smallint DEFAULT 0,     -- Cubic meters required, 0 = optional, -1 = not allowed, -2=limited
 field                                  smallint DEFAULT 0,     -- Count required, 0 = optional, -1 = not allowed.
 generator                              smallint DEFAULT -1,    -- Cubic meters required, 0 = optional, -1 = not allowed.
 guardPost                              smallint DEFAULT -1,    -- Count required, 0 = optional, -1 = not allowed.
 homeLarge                              smallint DEFAULT 0,     -- Count required, 0 = optional, -1 = not allowed.
 homeMedium                             smallint DEFAULT 0,     -- Count required, 0 = optional, -1 = not allowed.
 homeSmall                              smallint DEFAULT 0,     -- Count required, 0 = optional, -1 = not allowed.
 livestockPost                          smallint DEFAULT 0,     -- Count required, 0 = optional, -1 = not allowed.
 lounge                                 smallint DEFAULT 0,     -- Count required, 0 = optional, -1 = not allowed.
 office                                 smallint DEFAULT 0,     -- Count required, 0 = optional, -1 = not allowed.
 parkGround                             smallint DEFAULT 0,     -- Count required, 0 = optional, -1 = not allowed.
 parkSpace                              smallint DEFAULT 0,     -- Count required, 0 = optional, -1 = not allowed.
 parkWater                              smallint DEFAULT 0,     -- Count required, 0 = optional, -1 = not allowed.
 sensor                                 smallint DEFAULT -1,    -- Cubic meters required, 0 = optional, -1 = not allowed.
 shield                                 smallint DEFAULT -1,    -- Cubic meters required, 0 = optional, -1 = not allowed.
 shop                                   smallint DEFAULT 0,     -- Count required, 0 = optional, -1 = not allowed.
 store                                  smallint DEFAULT 0,     -- Count required, 0 = optional, -1 = not allowed.
 surgery                                smallint DEFAULT -1,    -- Count required, 0 = optional, -1 = not allowed.
 transporter                            smallint DEFAULT -1,    -- Count required, 0 = optional, -1 = not allowed.
 turret                                 smallint DEFAULT -1,    -- Count required, 0 = optional, -1 = not allowed.
 vehicleLauncher                        smallint DEFAULT -1,    -- Count required, 0 = optional, -1 = not allowed.
 vehicleRecovery                        smallint DEFAULT -1,    -- Count required, 0 = optional, -1 = not allowed.
 weaponBay                              smallint DEFAULT -1,    -- Count required, 0 = optional, -1 = not allowed.
 note                                   text,
 doesWhat                               text                    -- Appears on UI to briefly say what this does.
);
GRANT ALL ON b_Industry TO audog;
INSERT INTO b_Industry (id,name,storage,capacitor,emp, structure) VALUES (1,'Town Square',10,5,-5, 0);
INSERT INTO b_Industry (id,name, structure,field,arena,auditorium,battery,cargoStorage,homeLarge,homeMedium,homeSmall,livestockPost,lounge,office,parkGround,parkSpace,parkWater,store,shop,doesWhat) VALUES (2,'Road', 8,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,'Connects Buildings and Roads');
INSERT INTO b_Industry (id,name,emp, auditorium,office,note,doesWhat) VALUES (3,'Design Studio',-2, 1,1,'Each auditorium can create a separate designer instance.','Accesses On Line Designer');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,store,note,doesWhat) VALUES (4,'Warehouse',-1,200, 1,1,-1,'Imports a specific dry commodity until full.','Stores Products for Bulk Trade');
INSERT INTO b_Industry (id,name, structure,field,arena,auditorium,battery,cargoStorage,homeLarge,homeMedium,homeSmall,livestockPost,lounge,office,parkGround,parkSpace,parkWater,store,shop,doesWhat) VALUES (5,'Landscape', 0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,'Modifies Terrain and Plants');
INSERT INTO b_Industry (id,name,emp, structure) VALUES (10,'Military Flag',-5, 0);
INSERT INTO b_Industry (id,name,emp, buildRule,cargoStorage,guardPost,weaponBay,store,doesWhat) VALUES (11,'Military Weapon System',-1, 2,1,0,1,-1,'Attacks Enemy Units');
INSERT INTO b_Industry (id,name,emp, buildRule,battery,guardPost,shield,store,note,doesWhat) VALUES (12,'Military Shield Projector',-1, 2,0,0,1,-1,'Shield volume determines radius of shield bubble. Battery size increases power available to absorb damage.','Defends Buildings from Attack');
INSERT INTO b_Industry (id,name, structure,arena,auditorium,cargoStorage,field,homeLarge,homeMedium,homeSmall,livestockPost,lounge,office,parkGround,parkSpace,parkWater,shop,store,doesWhat) VALUES (13,'Street Light', 13,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,'Illuminates an area at night.');
INSERT INTO b_Industry (id,name,emp, buildRule,homeSmall,guardPost,store,note,doesWhat) VALUES (18,'Space Marine Barracks',-5, 2,2,0,-1,'Provides homes and training for space marines. Homes are filled by ground troops.','Houses Space Marines');
INSERT INTO b_Industry (id,name,emp, buildRule,guardPost,store,doesWhat) VALUES (19,'Military Guard Post',-5, 2,1,-1,'Stations Troops');
INSERT INTO b_Industry (id,name,pop,emp, homeSmall,store,note,doesWhat) VALUES (20,'House',2,-1, 1,-1,'Provides homes for citizens. Homes are filled by indigenous people, air/space travellers, and births.','Houses Citizens');
INSERT INTO b_Industry (id,name,pop,emp, structure) VALUES (21,'Condominium',3,-2, 0);
INSERT INTO b_Industry (id,name,pop,emp, homeSmall,office,note,doesWhat) VALUES (22,'Apartment',4,-1, 2,1,'Provides homes for citizens. Homes are filled by indigenous people, air/space travellers, and births.','Houses Citizens');

INSERT INTO b_Industry (id,name,emp, buildRule,homeSmall,guardPost,store,note,doesWhat) VALUES (27,'Military Barracks',-5, 2,2,0,-1,'Provides homes and training for ground troops. Homes are filled by the civilian population, indigenous population, and air/space travellers.','Houses Ground Troops');
INSERT INTO b_Industry (id,name,emp, buildRule,cargoStorage,guardPost,homeMedium,office,sensor,transporter,store,doesWhat) VALUES (28,'Military Headquarters',-5, 1,1,0,1,1,0,0,-1,'Founds and Manages a Military Base');
INSERT INTO b_Industry (id,name,emp, buildRule,guardPost,sensor,store,note,doesWhat) VALUES (29,'Military Radar',-5, 2,0,1,-1,'Provides air traffic control services within its range. Sensor volume determines radar range.','Tracks Targets for Weapon Systems and Gun Towers');
-- Agriculture
INSERT INTO b_Industry (id,name,pop,emp,storage, field,cargoStorage,homeSmall,livestockPost,doesWhat) VALUES (30,'Farm',2,1,10, 1,1,1,2,'Makes Products from Plants and Animals');
INSERT INTO b_Industry (id,name,emp,storage, field,cargoStorage,office,doesWhat) VALUES (31,'Logging Camp',1,10, 1,1,1,'Harvests Logs');
INSERT INTO b_Industry (id,name,pop,emp,storage, field,cargoStorage,homeSmall,doesWhat) VALUES (32,'Orchard',2,1,10, 1,1,1,'Makes Products from Trees');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (34,'Wharf',1,10, 1,1,0,'Harvests Fish, Connects to All Wharves and Military Docks on the World');
-- Agriculture Dependent
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (35,'Dairy',1,10, 1,1,1,'Makes Dairy Products');
INSERT INTO b_Industry (id,name,emp, structure,field,arena,auditorium,battery,cargoStorage,homeLarge,homeMedium,homeSmall,livestockPost,lounge,office,parkGround,parkSpace,parkWater,store,shop,doesWhat) VALUES (36,'Aerial Platform',-1, 6,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,'Provides aerial terrain for construction of buildings that fit on top. May encapsulate an environment for construction of buildings that fit inside. Encapsulated atmosphere is breathable but it cannot be harvested.');
INSERT INTO b_Industry (id,name,emp, structure,field,arena,auditorium,battery,cargoStorage,homeLarge,homeMedium,homeSmall,livestockPost,lounge,office,parkGround,parkSpace,parkWater,store,shop,doesWhat) VALUES (37,'Sea Platform',-1, 5,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,'Provides floating terrain on water, lava, or supercooled liquid, for construction of buildings that fit on top. May encapsulate an environment for construction of buildings that fit inside. Encapsulated atmosphere is breathable but it cannot be harvested.');
INSERT INTO b_Industry (id,name,emp, structure,field,arena,auditorium,battery,cargoStorage,homeLarge,homeMedium,homeSmall,livestockPost,lounge,office,parkGround,parkSpace,parkWater,store,shop,doesWhat) VALUES (38,'Ground Platform',-1, 7,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,'Provides ground terrain on land and under seas of water or supercooled liquid, for construction of buildings that fit on top. May encapsulate an environment for construction of buildings that fit inside. Encapsulated atmosphere is breathable but it cannot be harvested.');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (40,'Bakery',1,10, 1,1,1,'Bakes Food');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (41,'Brewery',1,10, 1,1,1,'Brews Beer');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (42,'Distillery',1,10, 1,1,1,'Distills Alcohol');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (43,'Grist Mill',1,10, 1,1,1,'Grinds Grain');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (44,'Lumber Mill',1,10, 1,1,1,'Saws Logs');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (45,'Meat Processing Plant',1,10, 1,1,1,'Butchers Meat');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (46,'Sugar Mill',1,10, 1,1,1,'Makes Sugar');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (47,'Textile Mill',1,10, 1,1,1,'Weaves Textiles');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (48,'Winery',1,10, 1,1,1,'Ferments Wine');
-- Mining
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (50,'Mine',1,10, 1,0,0,'Digs Rocky Geological Resources');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (51,'Well',1,10, 1,1,1,'Drills Liquid Geological Resources');
INSERT INTO b_Industry (id,name,emp,storage, structure,cargoStorage,office,shop,doesWhat) VALUES (52,'Drilling Rig',1,10, 9,1,1,1,'Drills Liquid Geological Resources');
-- Power Generation
INSERT INTO b_Industry (id,name,emp,capacitor, battery,cargoStorage,generator,office,store,doesWhat) VALUES (60,'Coal Power Plant',1,40, 1,1,1,1,-1,'Generates Electricity');
INSERT INTO b_Industry (id,name,emp,capacitor, battery,cargoStorage,generator,office,store,doesWhat) VALUES (61,'Gasoline Power Plant',1,40, 1,1,1,1,-1,'Generates Electricity');
INSERT INTO b_Industry (id,name,emp,capacitor, battery,generator,office,store,doesWhat) VALUES (62,'Hydroelectric Power Plant',1,80, 1,1,1,-1,'Generates Electricity');
INSERT INTO b_Industry (id,name,emp,capacitor, battery,cargoStorage,generator,office,store,doesWhat) VALUES (63,'Hydrogen Power Plant',1,60, 1,1,1,1,-1,'Generates Electricity');
INSERT INTO b_Industry (id,name,emp,capacitor, battery,cargoStorage,generator,office,store,doesWhat) VALUES (64,'Natural Gas Power Plant',1,60, 1,1,1,1,-1,'Generates Electricity');
INSERT INTO b_Industry (id,name,emp,capacitor, battery,generator,office,store,doesWhat) VALUES (66,'Solar Power Plant',1,30, 1,1,1,-1,'Generates Electricity');
INSERT INTO b_Industry (id,name,emp,capacitor, battery,generator,office,store,doesWhat) VALUES (67,'Wind Power Plant',1,30, 1,1,1,-1,'Generates Electricity');
INSERT INTO b_Industry (id,name,emp,capacitor, battery,cargoStorage,generator,office,store,doesWhat) VALUES (68,'Nuclear Power Plant',1,100, 1,1,1,1,-1,'Generates Electricity and Makes Plutonium'); -- This one makes plutonium and uses nuclear fuel rods.
-- Factories
INSERT INTO b_Industry (id,name,emp, cargoStorage,office,parkSpace,shop,doesWhat) VALUES (70,'Aircraft Factory',1, 1,3,1,3,'Makes Air and Space Vehicles');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (71,'Armorer',1,20, 1,1,1,'Makes Armor and Space Suits');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (72,'Carpenter',1,20, 1,1,1,'Makes Tools and Products');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (73,'Clothing Factory',1,20, 1,1,1,'Sews Clothing and Textile Products');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (74,'Computer Factory',1,20, 1,1,1,'Makes Computers');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (75,'Electronics Factory',1,20, 1,1,1,'Makes Electronic Devices');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (76,'Explosives Factory',1,20, 1,1,1,'Makes Explosives');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (77,'Weapon Smith',1,20, 1,1,1,'Makes Weapons');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (78,'Jeweler',1,20, 1,1,1,'Makes Jewelry and Wearables');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (79,'Machine Shop',1,20, 1,1,1,'Makes Parts and Tools');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (80,'Munitions Factory',1,20, 1,1,1,'Makes Ammunition');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (81,'Pharmaceutical Lab',1,20, 1,1,1,'Makes Drugs and Medical Supplies');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (82,'Plastics Factory',1,20, 1,1,1,'Makes Plastic and Plastic Products');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (83,'Refinery',1,20, 1,1,1,'Harvests Atmosphere, Refines Chemicals');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (84,'Rocket Motor Factory',1,20, 1,1,1,'Makes Rocket Motors');
INSERT INTO b_Industry (id,name,emp, cargoStorage,office,parkWater,shop,doesWhat) VALUES (85,'Ship Yard',1, 1,3,1,3,'Makes Boats and Submarines');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (86,'Smelter',1,20, 1,1,1,'Makes Metal');
INSERT INTO b_Industry (id,name,emp, cargoStorage,office,shop,doesWhat) VALUES (87,'Spacecraft Factory',1, 1,4,4,'Makes Spacecraft');
INSERT INTO b_Industry (id,name,emp, cargoStorage,office,parkGround,shop,doesWhat) VALUES (88,'Vehicle Factory',1, 1,3,1,3,'Makes Ground Vehicles');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (89,'Treatment Plant',1,10, 1,1,1,'Harvests Environmental Liquids');
INSERT INTO b_Industry (id,name,emp,storage, arena,auditorium,cargoStorage,homeLarge,homeMedium,homeSmall,livestockPost,lounge,office,shop,store,doesWhat) VALUES (90,'Nuclear Fuel Plant',1,10, -1,-1,1,-1,-1,-1,-1,-1,1,1,-1,'Makes Nuclear Fuel');
-- Retail
INSERT INTO b_Industry (id,name,emp, buildRule,cargoStorage,office,store,doesWhat) VALUES (100,'Bank',1, 5,1,1,-1,'Mints Money, Enables Tax Collection');
INSERT INTO b_Industry (id,name,emp, office,note,doesWhat) VALUES (101,'Broker',1, 2,'Each office can process one simultaneous shipment.','Buys and Sells Cargo, Sends Shipments to Other Solar Systems');
INSERT INTO b_Industry (id,name,emp, lounge,office,note,doesWhat) VALUES (102,'Casino',2, 1,1,'Each lounge accommodates up to 200 gamblers.','Gambling and Entertainment');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,store,note,doesWhat) VALUES (103,'Grocery',1,5, 1,1,1,'Each store accommodates up to 100 customers.','Sells Food');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,store,note,doesWhat) VALUES (104,'Retail Store',1,5, 1,1,1,'Each store accommodates up to 55 customers.','Sells Products');
INSERT INTO b_Industry (id,name,emp,storage, lounge,cargoStorage,office,note,doesWhat) VALUES (105,'Cantina',1,2, 1,1,1,'Each lounge accommodates up to 50 customers.','Eating, Drinking, Relaxation');
-- Airport/Spaceport Facilities
INSERT INTO b_Industry (id,name,emp, office,sensor,note,doesWhat) VALUES (109,'Airport Radar',1, 1,1,'Provides air traffic control services within its range. Sensor volume determines radar range.','Detects Units');
INSERT INTO b_Industry (id,name,emp, cargoStorage,office,shop,doesWhat) VALUES (110,'Airport Repair Shop',1, 1,1,1,'Repairs Spacecraft and Vehicles, Refits Spacecraft Systems');
INSERT INTO b_Industry (id,name,emp, lounge,cargoStorage,office,sensor,transporter,vehicleLauncher,vehicleRecovery,note,doesWhat) VALUES (111,'Airport Terminal',1, 0,1,2,0,0,0,0,'Imports hydrogen and gasoline. Imports air in harsh environments. Boards/debarks officers, crew, troops and passengers to/from fleet and company spacecraft.','Connects to All Airport Terminals in the Solar System, Loads Officers, Crew, Troops and Passengers');
INSERT INTO b_Industry (id,name,emp, arena,office,note,doesWhat) VALUES (112,'Arena',1, 1,1,'Each arena accommodates up to 175 spectators.','Hosts Virtual Arena Events');
INSERT INTO b_Industry (id,name,emp, field,cargoStorage,livestockPost,office,note,doesWhat) VALUES (113,'Zoo',1, 1,1,1,1,'Each field accommodates up to 150 visitors.','Displays Animals');
INSERT INTO b_Industry (id,name,emp, field,arena,auditorium,battery,homeLarge,homeMedium,homeSmall,livestockPost,lounge,office,parkGround,parkSpace,parkWater,store,shop,note,doesWhat) VALUES (114,'Park',0, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,'Each field accommodates up to 90 visitors.','Relaxation Space');
-- Government/Health
INSERT INTO b_Industry (id,name,emp, homeSmall,office,store,note,doesWhat) VALUES (119,'Private Security Contractor',1, 2,1,-1,'Provides homes and training for company troops. Homes are filled from the civilian population.','Houses Troops for Company Ships');
INSERT INTO b_Industry (id,name,emp, auditorium,office,note,doesWhat) VALUES (120,'Church',-2, 1,1,'Each auditorium accommodates up to 45 churchgoers.','Community Congregation Place');
INSERT INTO b_Industry (id,name,emp, office,surgery,store,note,doesWhat) VALUES (121,'Hospital',1, 1,1,-1,'Each office accommodates up to 80 patients.','Heals');
INSERT INTO b_Industry (id,name,emp, office,store,note,doesWhat) VALUES (122,'Fire Station',1, 2,-1,'Fire fighting potential is equal to the number of offices.','Fights Fires');
INSERT INTO b_Industry (id,name,emp, buildRule,cargoStorage,auditorium,office,sensor,transporter,store,note,doesWhat) VALUES (123,'Capitol',1, 3,1,1,1,0,0,-1,'Creates a city with a small jurisdiction. Buildings in its jurisdiction become part of the city. Add police stations to expand city jurisdiction.','Founds and Manages a City');
INSERT INTO b_Industry (id,name,emp, buildRule,office,store,note,doesWhat) VALUES (124,'Police Station',1, 4,1,-1,'Police expand the jurisdiction of a city. Police station enforcement radius increases with the number of offices in its blueprint.','Expands City Jurisdiction');
INSERT INTO b_Industry (id,name,emp, auditorium,homeSmall,office,note,doesWhat) VALUES (125,'University',1, 1,2,1,'Each auditorium accommodates up to 70 students. Provides homes and training for company officers and crew. Homes are filled from the civilian population. ','Researches Patents, Houses Company Officers and Crew');
INSERT INTO b_Industry (id,name,emp, office,store,doesWhat) VALUES (126,'Hangar',1, 1,-1,'Stores Spacecraft');
INSERT INTO b_Industry (id,name,emp, office,doesWhat) VALUES (127,'Observatory',1, 1,'Provides Astronomical Information');
INSERT INTO b_Industry (id,name,emp, buildRule,cargoStorage,guardPost,turret,store,doesWhat) VALUES (128,'Military Gun Tower',-1, 2,1,0,1,-1,'Attacks Enemy Units');
INSERT INTO b_Industry (id,name,emp, structure,field,arena,auditorium,battery,cargoStorage,homeLarge,homeMedium,homeSmall,livestockPost,lounge,office,parkGround,parkSpace,parkWater,store,shop,doesWhat) VALUES (129,'Biodome',-1, 1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,'Encapsulates an environment for construction of buildings that fit inside. Creates vegetation and trees for farms and logging camps. Atmosphere is breathable but it cannot be harvested.');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,doesWhat) VALUES (130,'Mass Media',1,1, 1,2,'Does Nothing');
INSERT INTO b_Industry (id,name, structure,field,arena,auditorium,battery,cargoStorage,homeLarge,homeMedium,homeSmall,livestockPost,lounge,office,parkGround,parkSpace,parkWater,store,shop,doesWhat) VALUES (131,'Fence/Wall', 4,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,'Blocks Movement');
INSERT INTO b_Industry (id,name, structure,field,arena,auditorium,battery,cargoStorage,homeLarge,homeMedium,homeSmall,livestockPost,lounge,office,parkGround,parkSpace,parkWater,store,shop,doesWhat) VALUES (132,'Bridge', 2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,'Crosses Rivers and Terrain');
INSERT INTO b_Industry (id,name, structure,field,arena,auditorium,battery,cargoStorage,homeLarge,homeMedium,homeSmall,livestockPost,lounge,office,parkGround,parkSpace,parkWater,store,shop,doesWhat) VALUES (133,'Tunnel', 12,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,'Passes Through Terrain');
INSERT INTO b_Industry (id,name,emp, buildRule,cargoStorage,guardPost,office,sensor,transporter,vehicleLauncher,vehicleRecovery,store,note,doesWhat) VALUES (134,'Military Airport Terminal',1, 2,1,0,1,0,0,0,0,-1,'Imports hydrogen and gasoline. Imports air in harsh environments. Boards/debarks officers, crew and troops to/from fleet spacecraft.','Stations Military Air and Space Vehicles, Connects to All Airport Terminals in the Solar System, Loads Fleet Officers, Crew and Troops');
INSERT INTO b_Industry (id,name,emp, buildRule,cargoStorage,guardPost,office,parkGround,store,doesWhat) VALUES (135,'Military Motor Pool',1, 2,1,0,1,1,-1,'Stations Military Ground Vehicles');
INSERT INTO b_Industry (id,name,emp, buildRule,cargoStorage,guardPost,office,parkWater,store,doesWhat) VALUES (136,'Military Dock',1, 2,1,0,1,1,-1,'Stations Military Water Vehicles, Connects to All Military Docks and Wharves on the World');
INSERT INTO b_Industry (id,name,emp,storage, field,arena,auditorium,battery,cargoStorage,homeLarge,homeMedium,homeSmall,livestockPost,lounge,office,parkGround,parkSpace,parkWater,store,shop,note,doesWhat) VALUES (137,'Shed',-1,100, -1,-1,-1,-1,0,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,'Imports a small amount of tools, spare parts, building materials, and other miscellaneous items until full.','Stores Miscellaneous Items');
INSERT INTO b_Industry (id,name,emp,storage, field,arena,auditorium,battery,cargoStorage,homeLarge,homeMedium,homeSmall,livestockPost,lounge,office,parkGround,parkSpace,parkWater,store,shop,note,doesWhat) VALUES (138,'Storage Tank',-1,100, -1,-1,-1,-1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,'Imports a specific liquid or gas commodity until full.','Stores Products for Bulk Trade');
INSERT INTO b_Industry (id,name,emp, buildRule,auditorium,homeSmall,office,note,doesWhat) VALUES (139,'Star Fleet Academy',1, 2,1,2,1,'Provides homes and training for fleet officers and crew. Homes are filled by ground troops.','Houses Fleet Officers and Crew');
INSERT INTO b_Industry (id,name,emp,storage, cargoStorage,office,shop,doesWhat) VALUES (140,'Paper Mill',1,10, 1,1,1,'Makes Paper and Paper Products');
SELECT * FROM b_Industry;

-- Manufacturing components are required for each manufacturing process in
-- b_ManufacturingProcess.
-- class DbManufacturingComponent
DROP TABLE b_ManufacturingComponent;
CREATE TABLE b_ManufacturingComponent
(
  id               serial primary key unique,
  idmp             integer,                      -- Manufacturing process to which this is related.
  idc              integer,                      -- Commodity consumed by this process.
  consumed         integer,                      -- Quantity consumed by this process.
  ide              smallint                      -- Effect of this component on the process. CE_ values in dbcommodity.h
                                                 -- 1=None, 2=DecreasesTime, 3=DecreasesMaterials, 4=RequiredQ, 5=Required, 6=IncreasesQuantity
);
GRANT ALL ON b_ManufacturingComponent TO audog;
GRANT ALL ON b_ManufacturingComponent_id_seq TO audog;
SELECT * FROM b_ManufacturingComponent;

-- Manufacturing processes are employed by industries for doing what they do.
-- class DbManufacturingProcess
-- NOTE: Manufacturing components are limited to 6 different modifiers. That is
--       commodities that either have 'consumes == 0' or 'ide != 5'.
DROP TABLE b_ManufacturingProcess;
CREATE TABLE b_ManufacturingProcess
(
  id               integer primary key unique,
  name             text,                         -- The name of this process.
  idc              smallint,                     -- Commodity produced by this process.
  idi              smallint,                     -- Industry that can invoke this process. (b_Industry.id)
  qty              smallint,                     -- Quantity produced by this process
  labor            integer,                      -- Labor required. LABOR_PER_SEC is 8 units per second.
  craft            smallint DEFAULT 2,           -- 0=no, 1=yes, 2=when in industry
  isdefault        boolean DEFAULT 'f',          -- true if a process is the default for its industry.
  nat              boolean DEFAULT 't'           -- When natural resource is produced, true to average in local natural res quality.
);
GRANT ALL ON b_ManufacturingProcess TO audog;
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (1,'Hydrogen using Atmosphere',44,83,1,24,0,'t'); -- hydrogen
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1,356,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (2,'Air using Atmosphere',5,83,2,24,0); -- air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (2,357,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (2,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (2,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (2,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (3,'Helicopter',6,70,1,100,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (3,358,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (3,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (3,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (3,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (3,50,3,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (3,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (3,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (3,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (3,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (3,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (3,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (4,'Space Rocket',7,70,1,350,0,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (4,359,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (4,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (4,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (4,5,1,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (4,50,3,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (4,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (4,64,1,5); -- Large rocket motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (4,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (4,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (4,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (4,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (4,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (5,'Space Fighter F1',8,70,1,500,0); -- Space Fighter F1
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (5,360,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (5,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (5,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (5,5,1,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (5,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (5,50,4,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (5,51,6,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (5,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (5,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (5,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (5,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (5,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (5,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (5,226,1,5); -- Grav Coupling for G-drive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (6,'Space Transport',9,70,1,450,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (6,361,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (6,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (6,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (6,5,1,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (6,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (6,50,8,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (6,51,15,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (6,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (6,79,6,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (6,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (6,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (6,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (6,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (6,226,2,5); -- Grav Couplings for G-drive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (7,'Dynamite',25,80,4,9,1,'t'); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (7,362,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (7,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (7,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (7,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (7,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (8,'Heavy Arms Ammo Pi',11,80,1,62); -- heavy arms ammo
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (8,363,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (8,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (8,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (8,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (8,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (8,51,2,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (9,'Plastic Explosive',13,80,1,8,1); -- Plastic Explosives
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (9,364,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (9,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (9,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (9,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (9,61,2,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (10,'Bomb Bl Black Cat using Plastic Explosive',12,80,1,64); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (10,365,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (10,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (10,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (10,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (10,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (10,13,100,5); -- Plastic Explosives
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (11,'Vegetables',81,30,1,24,0,'t'); -- Vegetable
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (11,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (11,28,0,6); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (11,29,1,6); -- Fertilizer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (11,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (11,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (11,114,0,6); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (12,'Fertilizer using Atmosphere',29,83,1,25,0,'f'); -- Fertilizer using Haber process
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (12,366,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (12,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (12,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (12,44,1,5); -- Hydrogen
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (12,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (13,'Beans',15,30,1,24,0); -- Beans
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (13,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (13,28,0,6); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (13,29,1,6); -- Fertilizer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (13,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (13,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (13,114,0,6); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (14,'Fertilizer using Air',29,83,1,25,0,'f'); -- Fertilizer using Haber process
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (14,367,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (14,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (14,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (14,44,1,5); -- Hydrogen
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (14,5,1,5); -- Air
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,isdefault) VALUES (15,'Beer',16,41,10,10,'t'); -- Beer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (15,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (15,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (15,38,1,5); -- Grain
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (15,43,1,5); -- Hops
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (15,83,2,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,isdefault) VALUES (16,'Leather Armor',17,71,1,129,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (16,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (16,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (16,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (16,47,10,5); -- Leather
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (16,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (17,'Bomb Bl Black Cat using Nitrate Explosive',12,80,1,64); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (17,365,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (17,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (17,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (17,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (17,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (17,93,100,5); -- Nitrate Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (18,'Bomb Bl Black Cat using Dynamite',12,80,1,64); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (18,365,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (18,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (18,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (18,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (18,25,100,5); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (18,51,4,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,isdefault) VALUES (19,'Large Rocket Motor using Nitrate Explosive',64,84,1,440,'t'); -- Large Rocket motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (19,370,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (19,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (19,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (19,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (19,51,20,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (19,93,40,5); -- Nitrate Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,isdefault) VALUES (20,'Bread',18,40,10,2,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (20,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (20,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (20,26,1,5); -- Eggs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (20,34,1,5); -- Flour
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (20,52,1,5); -- Milk
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (20,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (21,'Candy',19,40,12,6); -- candy
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (21,380,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (21,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (21,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (21,52,1,5); -- Milk
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (21,74,1,5); -- Spices
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (21,78,1,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (21,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (22,'Plant Fiber',30,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (22,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (22,28,0,6); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (22,29,1,6); -- Fertilizer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (22,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (22,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (22,114,0,6); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (23,'Hydrogen using Natural Gas',44,83,1,25,0,'f'); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (23,381,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (23,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (23,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (23,57,1,5); -- Natural Gas
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (24,'Large Rocket Motor using Gun Powder',64,84,1,440); -- Large Rocket motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (24,370,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (24,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (24,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (24,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (24,40,40,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (24,51,20,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (25,'Flag',1,73,2,4,1,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (25,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (25,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (25,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (25,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (26,'Stone',76,50,2,10,0,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (26,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (26,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (26,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (26,25,1,6); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (26,82,0,6); -- SUV
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (26,114,0,2); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,isdefault) VALUES (27,'Computer',161,74,1,137,'t'); -- computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (27,383,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (27,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (27,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (27,27,5,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (27,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (27,63,4,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (27,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (27,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (28,'Motor Bike',133,88,1,150,0,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (28,384,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (28,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (28,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (28,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (28,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (28,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (28,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (28,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (28,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault,nat) VALUES (29,'Water',83,52,2,24,0,'t','f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (29,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (29,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (29,88,1,5); -- Water in Environment
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (30,'Large Rocket Motor using Petrochemical',64,84,1,440);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (30,370,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (30,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (30,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (30,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (30,51,20,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (30,61,40,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (31,'Medical Kit',73,81,1,3,1,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (31,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (31,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (31,62,1,5); -- Pharmaceuticals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (31,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (31,126,1,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault,nat) VALUES (32,'Water',83,51,2,24,0,'t','f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (32,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (32,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (32,88,1,5); -- Water in Environment
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (33,'Rifle Ammunition',75,80,6,14); -- Rifle ammo
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (33,386,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (33,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (33,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (33,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (33,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (33,51,2,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,isdefault) VALUES (34,'Whiskey',84,42,1,6,'t'); -- Whiskey
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (34,387,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (34,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (34,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (34,38,1,5); -- Grain
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (34,78,1,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (34,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (35,'Metal Armor',90,71,1,190);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (35,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (35,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (35,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (35,51,10,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (36,'Plastic Armor',91,71,1,140);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (36,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (36,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (36,63,10,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (36,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (37,'Flak Vest',92,71,1,40);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (37,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (37,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (37,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (37,79,10,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (38,'Oil',23,51,2,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (38,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (38,3,1,2); -- Electricity
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (39,'Crystals',24,50,2,30,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (39,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (39,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (39,25,1,6); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (39,82,0,6); -- SUV
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (39,114,0,2); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (40,'Pistol Ammunition',10,80,5,32); -- Pistol ammo
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (40,389,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (40,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (40,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (40,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (40,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (40,51,2,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (41,'Gun Powder',40,76,2,2,1,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (41,390,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (41,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (41,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (41,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (41,22,1,5); -- Coal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (41,53,1,5); -- Minerals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (41,93,1,5); -- Nitrate Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (42,'Hay',130,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (42,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (42,28,0,6); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (42,29,1,6); -- Fertilizer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (42,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (42,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (42,114,0,6); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (43,'Oil',23,52,2,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (43,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (43,3,1,2); -- Electricity
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (44,'Electricity using Coal',3,60,40,4,0,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (44,391,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (44,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (44,22,1,5); -- Coal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (44,114,0,5); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (45,'Electricity using Gasoline',3,61,40,4,0,'t'); -- electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (45,392,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (45,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (45,36,1,5); -- Gasoline
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (45,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (46,'Electricity using Hydrogen',3,63,60,4,0,'t'); -- electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (46,393,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (46,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (46,44,1,5); -- Hydrogen
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (46,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (47,'Electricity using Natural Gas',3,64,60,4,0,'t'); -- electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (47,394,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (47,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (47,57,1,5); -- Natural Gas
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (47,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (48,'Electricity using Sunlight',3,66,30,4,0,'t'); -- electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (48,395,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (48,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (48,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (49,'Electricity using Wind',3,67,30,4,0,'t'); -- electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (49,396,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (49,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (49,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,isdefault) VALUES (51,'Electronic Part',27,75,2,9,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (51,398,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (51,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (51,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (51,24,1,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (51,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (51,53,1,5); -- Minerals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (51,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (51,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (52,'Farm Machinery',28,88,1,1500,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (52,399,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (52,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (52,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (52,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (52,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (52,50,4,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (52,51,8,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (52,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (53,'Fruit',35,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (53,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (53,28,0,6); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (53,29,1,6); -- Fertilizer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (53,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (53,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (53,114,0,6); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,isdefault) VALUES (55,'Knife using Metal and Wood',116,77,1,30,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (55,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (55,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (55,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (55,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (55,86,1,5); -- Wood
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (56,'Knife using Metal',116,77,1,30);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (56,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (56,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (56,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (56,51,2,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (57,'Knife using Metal and Plastic',116,77,1,30);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (57,400,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (57,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (57,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (57,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (57,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (57,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,nat) VALUES (58,'Water using Air and Hydrogen',83,83,1,24,'f'); -- water
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (58,401,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (58,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (58,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (58,5,1,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (58,44,1,5); -- Hydrogen
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (59,'Pistol using Metal and Wood',31,77,1,144); -- Pistol
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (59,402,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (59,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (59,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (59,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (59,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (59,86,2,5); -- Wood
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (60,'Vodka',101,42,1,2); -- vodka
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (60,403,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (60,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (60,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (60,38,1,5); -- Grain
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (60,78,1,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (60,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (61,'Pistol using Metal',31,77,1,137); -- Pistol
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (61,402,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (61,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (61,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (61,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (61,51,3,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (62,'Pistol using Metal and Plastic',31,77,1,142); -- Pistol
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (62,402,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (62,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (62,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (62,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (62,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (62,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (63,'Rifle using Metal',32,77,1,216); -- Rifle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (63,406,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (63,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (63,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (63,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (63,51,4,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (64,'Кала́шников using Metal',292,77,1,216); -- AK47
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (64,407,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (64,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (64,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (64,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (64,51,4,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (65,'Rifle using Metal and Plastic',32,77,1,205); -- Rifle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (65,406,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (65,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (65,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (65,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (65,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (65,63,2,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (66,'Rifle using Metal and Wood',32,77,1,223); -- Rifle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (66,406,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (66,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (66,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (66,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (66,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (66,86,2,5); -- Wood
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (67,'Fish',33,34,1,24,0,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (67,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (67,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (67,88,0,5); -- Water in Environment needed but not consumed
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,isdefault) VALUES (68,'Flour',34,43,4,8,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (68,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (68,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (68,38,1,5); -- Grain
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (69,'Jeans and Shirt',21,73,2,15,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (69,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (69,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (69,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (69,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (70,'Fruit',35,32,1,24,0,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (70,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (70,28,0,6); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (70,29,1,6); -- Fertilizer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (70,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (70,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (70,114,0,6); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (71,'Кала́шников using Metal and Plastic',292,77,1,205);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (71,407,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (71,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (71,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (71,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (71,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (71,63,2,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (72,'Gasoline',36,83,2,24); -- gasoline
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (72,411,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (72,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (72,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (72,23,1,5); -- Oil
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (73,'Gems',37,50,1,100,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (73,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (73,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (73,25,1,6); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (73,82,0,6); -- SUV
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (73,114,0,2); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (74,'Plant Fiber',30,32,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (74,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (74,28,0,6); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (74,29,1,6); -- Fertilizer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (74,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (74,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (74,114,0,6); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (75,'Кала́шников using Metal and Wood',292,77,1,223); -- AK47
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (75,407,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (75,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (75,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (75,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (75,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (75,86,2,5); -- Wood
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (76,'Herbs',42,32,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (76,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (76,28,0,6); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (76,29,1,6); -- Fertilizer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (76,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (76,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (76,114,0,6); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (77,'ParcX Uniform',369,73,2,16,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (77,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (77,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (77,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (77,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (78,'Grain',38,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (78,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (78,28,0,6); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (78,29,1,6); -- Fertilizer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (78,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (78,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (78,114,0,6); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (79,'Capri and Halter',372,73,2,14,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (79,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (79,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (79,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (79,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (80,'Electricity using Water in Environment',3,62,80,4,0,'t'); -- electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (80,413,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (80,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (80,88,0,5); -- Water in Environment needed but not consumed
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (81,'Nitrate Explosive',93,76,1,14);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (81,414,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (81,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (81,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (81,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (81,29,4,5); -- Fertilizer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (82,'Leather Clothing',95,73,2,19,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (82,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (82,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (82,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (82,47,1,5); -- Leather
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (83,'Flak Helmet',41,71,1,30);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (83,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (83,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (83,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (83,79,2,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (84,'Leather Helmet',96,71,1,40);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (84,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (84,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (84,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (84,47,2,5); -- Leather
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (85,'Plastic Helmet',97,71,1,50); -- Plastic helmet
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (85,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (85,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (85,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (85,63,2,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (86,'Herbs',42,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (86,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (86,28,0,6); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (86,29,1,6); -- Fertilizer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (86,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (86,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (86,114,0,6); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (87,'Utility Pouch using Leather',293,73,1,3,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (87,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (87,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (87,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (87,47,1,5); -- Leather
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (88,'Utility Pouch using Textiles',293,73,1,3,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (88,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (88,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (88,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (88,79,1,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (89,'Hops',43,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (89,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (89,28,0,6); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (89,29,1,6); -- Fertilizer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (89,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (89,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (89,114,0,6); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (91,'Petrochemicals',61,83,1,24); -- Petrochemicals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (91,416,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (91,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (91,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (91,23,1,5); -- Oil
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,nat) VALUES (92,'Hydrogen using Water',44,83,1,24,'f'); -- hydrogen
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (92,417,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (92,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (92,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (92,83,2,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (93,'Grav Coupling',226,75,1,20); -- Grav Coupling
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (93,418,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (93,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (93,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (93,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (93,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (93,134,2,5); -- Eludium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (94,'Ice',45,50,2,5,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (94,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (94,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (94,25,1,6); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (94,82,0,6); -- SUV
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (94,114,0,2); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (95,'Cryo Heat Sink',227,79,1,24);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (95,419,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (95,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (95,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (95,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (95,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (95,142,2,5); -- Cryozine
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,isdefault) VALUES (96,'Gem Ring',99,78,2,77,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (96,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (96,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (96,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (96,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (96,160,1,5); -- Gold
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (97,'Crystal Necklace',46,78,6,8);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (97,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (97,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (97,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (97,24,1,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (97,160,1,5); -- Gold
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (98,'Gold Bracelet',98,78,6,9);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (98,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (98,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (98,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (98,160,1,5); -- Gold
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (99,'Fish Meat',100,45,1,3,1,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (99,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (99,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (99,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (99,33,1,5); -- Fish
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (100,'Animal Meat',49,45,1,4,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (100,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (100,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (100,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (100,14,1,5); -- Animal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (101,'Leather',47,45,1,11,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (101,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (101,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (101,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (101,14,1,5); -- Animal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,isdefault,craft) VALUES (102,'Lumber',48,44,2,4,'t',1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (102,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (102,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (102,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (102,86,1,5); -- Wood
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (103,'Magmium Knife using Magmium and Wood',151,77,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (103,420,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (103,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (103,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (103,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (103,86,1,5); -- Wood
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (103,146,1,5); -- Magmium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (104,'Magmium Knife using Magmium',151,77,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (104,420,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (104,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (104,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (104,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (104,146,2,5); -- Magmium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (105,'Magmium Knife using Magmium and Plastic',151,77,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (105,420,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (105,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (105,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (105,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (105,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (105,146,1,5); -- Magmium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (106,'Nuts',58,32,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (106,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (106,28,0,6); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (106,29,1,6); -- Fertilizer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (106,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (106,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (106,114,0,6); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (107,'Brown Uniform',472,73,2,16,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (107,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (107,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (107,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (107,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (108,'Io Tube',228,75,2,90);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (108,423,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (108,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (108,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (108,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (108,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (108,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (108,138,2,5); -- Ioplasma
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,isdefault) VALUES (109,'Mechanical Part',50,79,1,9,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (109,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (109,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (109,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (109,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,isdefault) VALUES (110,'Plastic',63,82,5,33,'t'); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (110,424,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (110,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (110,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (110,61,2,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,isdefault) VALUES (111,'Metal using Ore',51,86,1,4,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (111,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (111,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (111,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (111,59,1,5); -- Ore
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (112,'Grapes',39,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (112,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (112,28,0,6); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (112,29,1,6); -- Fertilizer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (112,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (112,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (112,114,0,6); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (113,'Grey Uniform',477,73,2,16,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (113,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (113,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (113,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (113,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (114,'Minerals',53,50,2,20,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (114,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (114,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (114,25,1,6); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (114,82,0,6); -- SUV
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (114,114,0,2); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (115,'Red Uniform',479,73,2,16,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (115,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (115,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (115,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (115,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (116,'Mine Machinery',54,0,1,1500,0); -- disabled
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (116,425,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (116,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (116,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (116,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (116,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (116,50,6,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (116,51,10,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (116,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (117,'Small Missile Bl Zipper using Dynamite',55,80,1,563); -- Small Missile
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (117,426,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (117,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (117,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (117,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (117,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (117,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (117,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (117,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (117,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (117,25,1,5); -- Dynamite
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (118,'Small Missile Bl Zipper using Plastic Explosive',55,80,1,563); -- Small Missile
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (118,426,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (118,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (118,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (118,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (118,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (118,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (118,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (118,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (118,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (118,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (119,'Small Missile Bl Zipper using Nitrate Explosive',55,80,1,563); -- Small Missile
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (119,426,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (119,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (119,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (119,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (119,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (119,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (119,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (119,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (119,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (119,93,1,5); -- Nitrate Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (120,'Large Missile Bl Thumper',56,80,1,1532);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (120,429,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (120,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (120,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (120,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (120,27,14,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (120,64,1,5); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (120,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (120,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (120,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (120,12,1,5); -- Bomb
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (121,'Natural Gas',57,51,2,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (121,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (121,3,1,2); -- Electricity
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (122,'Nuts',58,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (122,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (122,28,0,6); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (122,29,1,6); -- Fertilizer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (122,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (122,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (122,114,0,6); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (123,'Green Uniform',480,73,2,16,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (123,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (123,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (123,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (123,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (124,'Ore',59,50,2,15,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (124,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (124,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (124,25,1,6); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (124,82,0,6); -- SUV
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (124,114,0,2); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (125,'Blue Uniform',481,73,2,16,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (125,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (125,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (125,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (125,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (126,'Pastry',60,40,15,7); -- pastry
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (126,430,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (126,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (126,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (126,26,1,5); -- Eggs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (126,34,1,5); -- Flour
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (126,35,1,5); -- Fruit
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (126,52,1,5); -- Milk
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (126,58,1,5); -- Nuts
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (126,74,1,5); -- Spices
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (126,78,1,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (126,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,nat) VALUES (127,'Air using Water',5,83,2,24,'f'); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (127,431,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (127,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (127,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (127,83,2,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (128,'Pharmaceuticals using Herbs',62,81,1,8,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (128,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (128,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (128,42,4,5); -- Herbs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (128,128,0,2); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (129,'Pharmaceuticals using Minerals',62,81,1,9,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (129,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (129,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (129,53,1,5); -- Minerals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (129,128,0,2); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (130,'Pharmaceuticals using Petrochemicals',62,81,1,9);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (130,432,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (130,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (130,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (130,61,1,5); -- Petrochemicals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (130,128,0,2); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (131,'Mechanical Part using Plastic',50,82,1,9);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (131,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (131,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (131,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (131,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (132,'Spices',74,32,1,24,0); -- spices at orchard
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (132,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (132,28,0,6); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (132,29,1,6); -- Fertilizer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (132,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (132,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (132,114,0,6); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (133,'Teal Uniform',482,73,2,16,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (133,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (133,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (133,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (133,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (134,'Metal EV Suit',65,71,1,920);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (134,434,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (134,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (134,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (134,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (134,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (134,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (134,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (134,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (134,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (134,50,1,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (134,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (134,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (134,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (134,227,1,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (135,'Radioactives',66,50,2,40,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (135,435,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (135,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (135,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (135,25,1,6); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (135,82,0,6); -- SUV
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (135,114,0,2); -- Shovel
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (135,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (136,'Gold Uniform',483,73,2,16,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (136,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (136,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (136,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (136,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (137,'Small Rocket Bl Partypopper using Dynamite',67,80,1,473);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (137,436,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (137,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (137,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (137,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (137,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (137,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (137,25,1,5); -- Dynamite
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (138,'Small Rocket Bl Partypopper using Plastic Explosive',67,80,1,473);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (138,436,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (138,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (138,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (138,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (138,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (138,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (138,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (139,'Small Rocket Bl Partypopper using Nitrate Explosive',67,80,1,473);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (139,436,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (139,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (139,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (139,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (139,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (139,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (139,93,1,5); -- Nitrate Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (140,'Large Rocket Bl Thud',68,80,1,748);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (140,439,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (140,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (140,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (140,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (140,64,1,5); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (140,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (140,12,1,5); -- Bomb
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (141,'Small Rocket Motor using Gun Powder',69,84,1,130);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (141,440,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (141,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (141,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (141,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (141,40,10,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (141,51,10,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (142,'Small Rocket Motor using Petrochemicals',69,84,1,130);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (142,440,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (142,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (142,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (142,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (142,51,10,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (142,61,10,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (143,'Sailboat using Wood',132,85,1,200,1,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (143,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (143,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (143,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (143,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (143,48,7,5); -- Lumber
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (143,79,3,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (144,'Sailboat using Metal',132,85,1,200,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (144,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (144,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (144,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (144,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (144,51,7,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (144,79,3,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (145,'Sailboat using Plastic',132,85,1,200,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (145,442,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (145,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (145,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (145,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (145,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (145,63,7,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (145,79,3,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (146,'Tank',103,0,1,5000,0); -- disabled
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (146,443,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (146,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (146,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (146,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (146,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (146,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (146,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (146,50,4,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (146,51,10,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (146,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (146,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (147,'APC',104,88,1,2500,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (147,444,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (147,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (147,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (147,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (147,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (147,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (147,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (147,50,3,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (147,51,8,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (147,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (147,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (148,'Metal Helmet',105,71,1,60);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (148,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (148,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (148,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (148,51,2,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (149,'Metal EV Helmet',106,71,1,250);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (149,445,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (149,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (149,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (149,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (149,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (149,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (149,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (149,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (149,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (149,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (149,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (149,79,1,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (150,'Magmium EV Helmet',154,71,1,350);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (150,446,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (150,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (150,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (150,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (150,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (150,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (150,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (150,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (150,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (150,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (150,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (150,146,1,5); -- Magmium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (151,'Commercial Ship',70,0,1,4,0); -- disabled
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (151,447,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (151,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (151,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (151,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (151,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (151,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (151,27,3,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (151,48,25,5); -- Lumber
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (151,50,9,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (151,51,30,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (151,63,15,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (151,79,22,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (151,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (152,'Spices',74,30,1,24,0); -- spices at farm
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (152,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (152,28,0,6); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (152,29,1,6); -- Fertilizer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (152,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (152,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (152,114,0,6); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,isdefault) VALUES (153,'Sugar using Vegetables',78,46,2,8,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (153,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (153,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (153,81,2,5); -- Vegetable
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (153,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (154,'Sugar using Grain',78,46,3,2);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (154,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (154,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (154,38,2,5); -- Grain
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (154,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (155,'Pink Uniform',484,73,2,16,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (155,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (155,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (155,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (155,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (156,'Coal',22,50,2,10,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (156,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (156,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (156,25,1,6); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (156,82,0,6); -- SUV
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (156,114,0,2); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (157,'Battle Ship',71,0,1,4,0); -- disabled
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (157,448,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (157,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (157,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (157,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (157,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (157,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (157,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (157,48,20,5); -- Lumber
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (157,50,16,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (157,51,45,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (157,63,12,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (157,79,16,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (157,102,2,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (157,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (158,'Submarine',77,85,1,300,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (158,449,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (158,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (158,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (158,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (158,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (158,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (158,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (158,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (158,50,7,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (158,51,9,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (158,63,9,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (158,79,4,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (159,'Sugar using Grapes',78,46,2,11);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (159,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (159,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (159,39,1,5); -- Grapes
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (159,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (160,'Sugar using Fruit',78,46,2,11);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (160,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (160,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (160,35,1,5); -- Fruit
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (160,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (161,'Textiles using Plant Fiber',79,47,1,9,1,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (161,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (161,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (161,30,2,5); -- Plant Fiber
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (162,'Textiles using Plastic',79,47,3,13);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (162,450,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (162,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (162,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (162,63,2,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (163,'Hammer using Log',4,72,3,23,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (163,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (163,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (163,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (163,86,1,5); -- Log
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (164,'Hammer using Plastic',4,82,3,14);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (164,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (164,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (164,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (164,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (165,'Hammer using Metal',4,79,3,9);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (165,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (165,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (165,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (165,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (166,'EV Suit',80,71,1,608);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (166,452,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (166,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (166,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (166,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (166,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (166,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (166,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (166,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (166,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (166,79,1,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (167,'Torch',131,72,5,23,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (167,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (167,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (167,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (167,23,1,5); -- Oil
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (167,86,1,5); -- Log
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (168,'EV Helmet',72,71,1,120);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (168,453,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (168,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (168,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (168,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (168,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (168,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (168,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (168,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (168,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (168,79,1,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (169,'SUV',82,88,1,250,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (169,454,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (169,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (169,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (169,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (169,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (169,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (169,50,4,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (169,51,5,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (169,63,4,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (169,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (169,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault,nat) VALUES (170,'Water using Environment',83,89,1,12,0,'t','f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (170,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (170,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (170,88,1,5); -- Water in Environment
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (171,'Water using Ice',83,89,1,12,1,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (171,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (171,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (171,45,1,5); -- Ice
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (172,'Gin',115,42,1,4);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (172,455,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (172,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (172,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (172,38,1,5); -- Grain
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (172,74,1,5); -- Spices
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (172,78,1,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (172,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,isdefault) VALUES (173,'Wine using Grapes',85,48,1,3,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (173,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (173,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (173,39,2,5); -- Grapes
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (173,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (174,'Wine using Fruit',85,48,1,3);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (174,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (174,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (174,35,2,5); -- Fruit
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (174,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (175,'Log',86,31,1,12,0,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (175,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (175,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (175,4,0,2); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (176,'Heavy Weapon',102,77,1,40);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (176,456,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (176,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (176,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (176,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (176,50,1,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (176,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (176,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (176,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (177,'Rum',121,42,1,2);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (177,457,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (177,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (177,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (177,74,1,5); -- Spices
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (177,78,2,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (177,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (178,'Small Rocket Motor using Nitrate Explosive',69,84,1,130);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (178,440,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (178,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (178,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (178,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (178,51,10,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (178,93,10,5); -- Nitrate Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (179,'Hammer using Lumber',4,72,1,4,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (179,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (179,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (179,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (179,48,1,5); -- Lumber
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (180,'Hammer using Stone',4,72,2,10,1,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (180,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (180,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (180,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (180,76,1,5); -- Stone
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (181,'Tequila',122,42,1,8);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (181,459,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (181,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (181,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (181,42,3,5); -- Herbs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (181,78,1,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (181,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (182,'Crate using Lumber',107,72,1,4,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (182,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (182,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (182,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (182,48,1,5); -- Lumber
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (183,'Crate using Plastic',107,82,2,4);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (183,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (183,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (183,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (183,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (184,'Lantern',108,79,2,22);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (184,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (184,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (184,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (184,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (184,57,1,5); -- Natural Gas
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (185,'Lantern',108,75,2,11);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (185,461,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (185,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (185,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (185,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (185,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (185,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (186,'Knapsack using Leather',109,73,1,4,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (186,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (186,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (186,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (186,47,1,5); -- Leather
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (187,'Knapsack using Textiles',109,73,1,4,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (187,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (187,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (187,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (187,79,1,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (188,'Backpack using Leather',110,73,1,8,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (188,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (188,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (188,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (188,47,2,5); -- Leather
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (189,'Backpack using Textiles',110,73,1,8,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (189,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (189,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (189,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (189,79,2,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (190,'Lumenite',111,50,1,120,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (190,462,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (190,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (190,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (190,25,1,6); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (190,82,0,6); -- SUV
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (190,114,0,2); -- Shovel
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (190,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (191,'Blank Disk using Metal',112,75,20,4);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (191,463,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (191,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (191,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (191,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (191,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (192,'Blank Disk using Plastic',112,75,20,4);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (192,463,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (192,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (192,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (192,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (192,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (193,'Antidote using Herbs',119,81,1,19);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (193,465,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (193,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (193,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (193,42,1,5); -- Herbs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (193,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (194,'Antidote using Minerals',119,81,1,19);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (194,466,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (194,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (194,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (194,53,1,5); -- Minerals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (194,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (195,'Antidote using Petrochemicals',119,81,1,19);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (195,467,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (195,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (195,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (195,61,1,5); -- Petrochemicals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (195,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (196,'Antitoxin using Animal Carcass',120,81,1,42);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (196,468,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (196,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (196,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (196,14,2,5); -- Animals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (196,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (197,'Antitoxin using Eggs',120,81,1,48);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (197,469,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (197,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (197,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (197,26,4,5); -- Eggs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (197,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (200,'Head',332,0,1,10,0); -- Head for cost calc
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (200,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (200,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (200,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (200,14,1,5); -- Animal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (201,'Textiles using Leather',79,47,2,12,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (201,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (201,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (201,47,1,5); -- Leather
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (202,'Shotgun using Metal',117,77,1,166); -- Shotgun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (202,470,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (202,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (202,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (202,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (202,51,4,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (203,'Shotgun using Metal and Plastic',117,77,1,171);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (203,470,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (203,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (203,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (203,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (203,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (203,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (204,'Shotgun using Metal and Wood',117,77,1,180);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (204,470,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (204,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (204,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (204,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (204,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (204,86,1,5); -- Wood
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (205,'Eludium',134,50,2,110,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (205,473,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (205,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (205,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (205,25,1,6); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (205,82,0,6); -- SUV
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (205,114,0,2); -- Shovel
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (205,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (206,'Shotgun Ammunition',118,80,8,6);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (206,474,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (206,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (206,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (206,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (206,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (206,51,2,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (207,'Fitting Wrench',123,82,3,14);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (207,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (207,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (207,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (207,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (208,'Fitting Wrench',123,79,3,9);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (208,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (208,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (208,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (208,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (209,'Glue Gun',124,79,3,22);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (209,476,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (209,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (209,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (209,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (209,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (209,57,1,5); -- Natural Gas
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (210,'Glue Gun',124,75,3,11); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (210,476,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (210,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (210,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (210,127,0,2); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (210,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (210,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (211,'Soldering Iron',127,79,2,22); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (211,478,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (211,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (211,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (211,123,0,2); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (211,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (211,57,1,5); -- Natural Gas
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (212,'Soldering Iron',127,75,2,11); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (212,478,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (212,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (212,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (212,127,0,2); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (212,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (212,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (213,'Screw Driver',125,82,5,14); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (213,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (213,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (213,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (213,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (214,'Screw Driver',125,79,5,9);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (214,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (214,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (214,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (214,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (215,'Sewing Needle',126,82,6,14);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (215,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (215,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (215,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (215,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (216,'Sewing Needle',126,79,6,9);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (216,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (216,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (216,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (216,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (217,'Tongs',128,82,1,14);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (217,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (217,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (217,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (217,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (218,'Tongs',128,79,1,9);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (218,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (218,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (218,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (218,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (219,'Wrench',129,82,4,14);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (219,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (219,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (219,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (219,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (220,'Wrench',129,79,4,9);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (220,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (220,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (220,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (220,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (221,'Sewing Needle using Log',126,72,6,23,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (221,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (221,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (221,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (221,86,1,5); -- Log
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (222,'Shovel using Log',114,72,2,23,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (222,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (222,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (222,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (222,86,1,5); -- Log
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (223,'Shovel',114,82,2,14);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (223,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (223,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (223,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (223,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (224,'Shovel',114,79,2,9);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (224,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (224,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (224,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (224,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (225,'Milk using Pasture',52,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (225,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (225,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (225,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (225,89,1,5); -- Vegetation
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (226,'Milk using Hay',52,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (226,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (226,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (226,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (226,130,1,5); -- Hay
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (227,'Grav Pack using Leather',294,73,1,12,2);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (227,485,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (227,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (227,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (227,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (227,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (227,135,0,5); -- Pliers
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (227,47,2,5); -- Leather
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (227,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (227,226,1,5); -- Grav Coupling
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (228,'Grav Pack using Textiles',294,73,1,12,2);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (228,485,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (228,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (228,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (228,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (228,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (228,135,0,5); -- Pliers
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (228,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (228,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (228,226,1,5); -- Grav Coupling
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (229,'Cheese using Pasture',20,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (229,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (229,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (229,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (229,89,1,5); -- Vegetation
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (230,'Cheese using Hay',20,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (230,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (230,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (230,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (230,130,1,5); -- Hay
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (231,'White Uniform',486,73,2,16,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (231,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (231,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (231,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (231,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (232,'Black Uniform',488,73,2,16,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (232,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (232,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (232,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (232,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (233,'Eggs using Pasture',26,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (233,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (233,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (233,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (233,89,1,5); -- Vegetation
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (234,'Eggs using Hay',26,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (234,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (234,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (234,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (234,130,1,5); -- Hay
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (235,'Orange Uniform',500,73,2,16,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (235,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (235,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (235,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (235,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (236,'Natural Gas',57,52,2,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (236,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (236,3,1,2); -- Electricity
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (237,'Fertilizer using Pasture',29,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (237,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (237,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (237,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (237,89,1,5); -- Vegetation
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (238,'Fertilizer using Hay',29,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (238,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (238,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (238,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (238,130,1,5); -- Hay
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (241,'Animal Carcass using Pasture',14,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (241,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (241,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (241,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (241,89,1,5); -- Vegetation
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (242,'Animal Carcass using Hay',14,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (242,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (242,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (242,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (242,130,1,5); -- Hay
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (245,'Pliers',135,82,2,14);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (245,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (245,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (245,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (245,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (246,'Pliers',135,79,2,9);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (246,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (246,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (246,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (246,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (247,'Torque Wrench',136,79,1,24);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (247,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (247,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (247,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (247,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (248,'Vulcanite',137,50,2,535,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (248,489,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (248,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (248,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (248,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (248,25,1,6); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (248,82,0,6); -- SUV
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (248,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (249,'Bolite',141,50,1,200,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (249,490,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (249,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (249,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (249,25,1,6); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (249,82,0,6); -- SUV
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (249,114,0,5); -- Shovel
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (249,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (250,'Ioplasma',138,83,1,200,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (250,491,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (250,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (250,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (250,79,0,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (250,149,0,5); -- Inferno Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (251,'Cryozine',142,83,1,200,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (251,492,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (251,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (251,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (251,79,0,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (251,150,0,5); -- Frigid Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (252,'Magmex',139,89,1,80,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (252,493,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (252,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (252,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (252,114,0,5); -- Shovel
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (252,128,0,5); -- Tongs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (252,147,0,5); -- Lava in Environment
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (253,'Myrathane',143,89,1,200,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (253,494,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (253,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (253,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (253,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (253,148,0,5); -- Supercooled Liquid in Environment
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (254,'Phlogiston',140,51,1,200,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (254,495,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (254,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (254,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (254,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (255,'Polytaride',144,51,1,200,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (255,496,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (255,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (255,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (255,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (256,'Vulcium',145,86,1,501);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (256,497,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (256,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (256,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (256,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (256,137,1,5); -- Vulcanite
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (257,'Magmium',146,86,1,47);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (257,498,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (257,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (257,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (257,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (257,139,1,5); -- Magmex
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (258,'Vulcium Knife using Vulcium and Wood',152,77,1,200);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (258,499,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (258,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (258,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (258,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (258,86,1,5); -- Wood
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (258,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (259,'Vulcium Knife using Vulcium',152,77,1,200);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (259,499,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (259,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (259,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (259,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (259,145,2,5); -- Vulcium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (260,'Vulcium Knife using Vulcium and Plastic',152,77,1,200);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (260,499,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (260,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (260,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (260,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (260,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (260,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (261,'Magmium EV Suit',153,71,1,1220);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (261,502,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (261,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (261,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (261,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (261,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (261,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (261,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (261,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (261,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (261,50,1,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (261,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (261,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (261,146,1,5); -- Magmium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (261,264,1,5); -- Magmium Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (262,'Vulcium EV Suit',155,71,1,1520);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (262,503,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (262,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (262,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (262,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (262,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (262,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (262,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (262,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (262,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (262,50,1,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (262,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (262,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (262,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (262,265,1,5); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (263,'Vulcium EV Helmet',156,71,1,450);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (263,504,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (263,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (263,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (263,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (263,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (263,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (263,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (263,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (263,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (263,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (263,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (263,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (264,'Grey BDU',408,73,2,20,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (264,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (264,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (264,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (264,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (265,'Khaki BDU',409,73,2,20,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (265,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (265,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (265,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (265,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (266,'Olive Drab BDU',410,73,2,20,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (266,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (266,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (266,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (266,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (267,'Phlogiston',140,52,1,200,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (267,495,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (267,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (267,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (267,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (268,'Polytaride',144,52,1,200,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (268,496,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (268,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (268,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (268,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (269,'Viathol',611,52,1,350,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (269,615,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (269,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (269,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (269,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (270,'Gold Ring',162,78,2,99);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (270,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (270,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (270,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (270,160,1,5); -- Gold
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (400,'Clamp',229,82,1,12);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (400,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (400,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (400,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (400,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (401,'Clamp',229,79,1,8);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (401,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (401,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (401,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (401,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (402,'Oscilloscope',230,75,1,80);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (402,506,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (402,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (402,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (402,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (402,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (402,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (402,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (403,'Space Fighter F2',231,70,1,700,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (403,507,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (403,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (403,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (403,5,2,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (403,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (403,50,8,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (403,51,10,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (403,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (403,102,2,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (403,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (403,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (403,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (403,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (403,226,2,5); -- Grav Coupling for G-drive
-- The next two processes are for mission data disks. The industry is 0 so they can't be manufactured directly.
-- They are here so the cost analyzer can determine their base price.
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (404,'Mission Disk using Metal',232,0,20,4,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (404,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (404,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (404,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (404,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (405,'Mission Disk using Plastic',232,0,20,4,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (405,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (405,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (405,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (405,63,1,5); -- Plastic
-- End of data disk bogus processes.
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (406,'Bone',233,45,1,4,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (406,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (406,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (406,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (406,14,1,5); -- Animal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (407,'Sewing Needle using Bone',126,72,2,11,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (407,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (407,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (407,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (407,233,1,5); -- Bone
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (408,'Log',86,32,1,24,0,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (408,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (408,28,0,6); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (408,29,1,6); -- Fertilizer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (408,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (408,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (408,114,0,6); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (409,'Brain',373,0,1,4,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (409,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (409,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (409,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (409,332,1,5); -- Head
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,isdefault) VALUES (410,'Command And Control Module',234,110,1,100,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (410,508,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (410,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (410,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (410,50,1,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (410,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (410,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (410,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (410,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (410,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (410,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (410,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (411,'Gravity Drive Module',235,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (411,509,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (411,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (411,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (411,50,6,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (411,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (411,51,6,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (411,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (411,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (411,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (411,226,2,5); -- Grav Coupling for G-drive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (411,135,0,5); -- Pliers
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (412,'Rocket Drive Module',236,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (412,510,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (412,50,6,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (412,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (412,51,8,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (412,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (412,64,2,5); -- Large rocket motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (412,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (412,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (412,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (413,'Metal Hull',237,0,1,100); -- Upgrade Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (413,27,5,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (413,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (413,50,5,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (413,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (413,51,5,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (413,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (413,63,5,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (413,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (414,'Life Support Module',238,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (414,512,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (414,5,2,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (414,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (414,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (414,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (414,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (414,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (414,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (414,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (414,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (414,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (414,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (414,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (415,'Hydrogen Power Plant Module',239,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (415,513,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (415,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (415,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (415,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (415,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (415,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (415,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (415,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (415,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (416,'Energy Shield Module',240,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (416,514,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (416,27,5,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (416,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (416,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (416,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (416,227,1,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (416,229,0,5); -- Clamp
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (417,'Kinetic Shield Module',241,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (417,515,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (417,27,5,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (417,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (417,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (417,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (417,264,1,5); -- Magmium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (417,229,0,5); -- Clamp
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (417,228,1,5); -- Io tube
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (417,230,0,5); -- Oscilloscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (418,'Cloaking Shield Module',242,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (418,516,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (418,27,5,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (418,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (418,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (418,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (418,265,5,5); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (418,229,0,5); -- Clamp
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (418,228,1,5); -- Io tube
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (418,230,0,5); -- Oscilloscope
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (418,269,5,5); -- Tesloid Dissipator
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (418,270,0,5); -- Nanopath Calibrator
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (419,'Surgery Medical Module',243,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (419,517,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (419,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (419,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (419,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (419,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (419,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (419,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (419,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (419,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (419,79,4,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (419,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (420,'Genetics Medical Module',244,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (420,518,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (420,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (420,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (420,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (420,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (420,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (420,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (420,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (420,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (420,266,4,5); -- Myrasplicer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (420,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (421,'Cybernetics Medical Module',245,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (421,519,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (421,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (421,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (421,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (421,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (421,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (421,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (421,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (421,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (421,268,4,5); -- Gigacell
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (421,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (422,'Personnel Transporter Module',246,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (422,520,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (422,27,3,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (422,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (422,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (422,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (422,228,1,5); -- Io tube
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (422,230,0,5); -- Oscilloscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (423,'Vehicle Transporter Module',247,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (423,521,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (423,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (423,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (423,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (423,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (423,228,2,5); -- Io tube
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (423,230,0,5); -- Oscilloscope
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (423,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (423,229,0,5); -- Clamp
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (424,'Energy Weapon Turret Module',248,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (424,522,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (424,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (424,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (424,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (424,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (424,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (424,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (424,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (424,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (424,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (424,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (424,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (424,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (424,227,1,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (424,229,0,5); -- Clamp
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (426,'Weapon Launcher Turret Module',250,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (426,523,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (426,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (426,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (426,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (426,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (426,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (426,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (426,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (426,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (426,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (426,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (426,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (426,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (428,'Energy Weapon Bay Module',252,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (428,524,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (428,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (428,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (428,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (428,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (428,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (428,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (428,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (428,229,0,5); -- Clamp

INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (430,'Weapon Launcher Bay Module',254,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (430,525,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (430,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (430,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (430,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (430,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (430,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (430,4,0,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (432,'Atmosphere Condenser Bay Module',256,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (432,526,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (432,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (432,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (432,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (432,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (432,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (432,4,0,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (433,'Prospector Bay Module',257,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (433,527,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (433,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (433,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (433,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (433,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (433,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (433,4,0,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (434,'Tractor Beam Bay Module',258,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (434,528,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (434,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (434,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (434,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (434,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (434,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (434,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (434,227,1,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (434,229,0,5); -- Clamp
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (434,228,2,5); -- Io tube
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (434,230,0,5); -- Oscilloscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (435,'Wormhole P FTL Drive Module',259,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (435,529,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (435,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (435,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (435,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (435,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (435,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (435,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (435,111,1,5); -- Lumenite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (435,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (436,'Warp 1 FTL Drive Module',260,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (436,530,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (436,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (436,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (436,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (436,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (436,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (436,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (436,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (436,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (437,'Ballistic Weapon Bay Module',261,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (437,531,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (437,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (437,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (437,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (437,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (437,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (437,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (437,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (437,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (438,'Ballistic Weapon Turret Module',262,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (438,532,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (438,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (438,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (438,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (438,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (438,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (438,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (438,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (438,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (438,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (438,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (438,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (438,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (439,'Sensor Module',263,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (439,533,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (439,27,3,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (439,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (439,50,1,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (439,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (439,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (439,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (440,'Magmium Heat Sink',264,79,1,50);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (440,534,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (440,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (440,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (440,146,1,5); -- Magmium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (440,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (440,142,2,5); -- Cryozine
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (441,'Vulcium Heat Sink',265,79,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (441,535,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (441,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (441,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (441,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (441,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (441,142,2,5); -- Cryozine
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (442,'Myrasplicer',266,81,1,140);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (442,536,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (442,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (442,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (442,24,0,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (442,143,2,5); -- Myrathane
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (442,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (443,'Microscope',267,75,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (443,537,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (443,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (443,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (443,24,1,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (443,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (443,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (443,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (443,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (443,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (444,'Gigacell',268,75,1,30);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (444,538,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (444,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (444,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (444,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (444,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (444,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (444,140,1,5); -- Phlogiston
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (445,'Tesloid Dissipator',269,75,1,1000);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (445,539,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (445,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (445,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (445,24,1,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (445,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (445,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (445,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (445,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (445,135,0,5); -- Pliers
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (445,141,2,5); -- Bolite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (445,270,0,5); -- Nanopath Calibrator
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (446,'Nanopath Calibrator',270,75,1,90);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (446,540,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (446,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (446,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (446,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (446,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (446,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (446,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (446,143,1,5); -- Myrathane
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (447,'Antiflux Particles',271,83,1,1000,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (447,113,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (447,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (447,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (447,269,0,5); -- Tesloid Dissipator
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (448,'Borexino Precipitate',272,83,1,2000,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (448,158,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (448,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (448,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (448,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (448,269,0,5); -- Tesloid Dissipator
-- The next few processes are for commodities that are collected from the environment by resource collecting spacecraft. The industry is 0 so they can't be manufactured directly.
-- They are here so the cost analyzer can determine their base price.
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (449,'Type O Preons',273,0,1,3000,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (449,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (449,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (449,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (450,'Type B Preons',274,0,1,4000,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (450,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (450,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (450,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (451,'Type A Preons',275,0,1,5000,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (451,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (451,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (451,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (452,'Type F Preons',276,0,1,6000,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (452,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (452,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (452,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (453,'Type G Preons',277,0,1,7000,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (453,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (453,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (453,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (454,'Type K Preons',278,0,1,8000,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (454,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (454,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (454,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (455,'Type M Preons',279,0,1,9000,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (455,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (455,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (455,87,0,5); -- Atmosphere Density
-- End of resource collector bogus processes.
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (456,'Magmium Hull',280,0,1,500); -- Upgrade Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (456,27,5,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (456,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (456,50,5,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (456,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (456,146,5,5); -- Magmium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (456,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (456,63,5,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (456,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (457,'Vulcium Hull',281,0,1,1000); -- Upgrade Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (457,27,5,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (457,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (457,50,5,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (457,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (457,145,5,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (457,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (457,63,5,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (457,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (458,'Paper using Lumber',282,140,10,10,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (458,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (458,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (458,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (458,48,1,5); -- Lumber
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (459,'Paper using Log',282,140,20,30,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (459,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (459,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (459,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (459,86,1,5); -- Log
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (460,'Particle Collector Bay Module',283,110,1,150);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (460,543,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (460,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (460,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (460,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (460,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (460,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (460,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (460,269,1,5); -- Tesloid Dissipator
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (460,270,0,5); -- Nanopath Calibrator
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (461,'DNA Sample Kit',284,81,10,12);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (461,544,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (461,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (461,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (461,79,1,5); -- Textile
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (461,267,0,5); -- Microscope
-- The next process is for DNA samples. The industry is 0 so they can't be manufactured directly.
-- They are here so the cost analyzer can determine their base price.
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (462,'DNA Sample',285,0,10,12,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (462,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (462,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (462,79,1,5); -- Textile
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (462,267,0,5); -- Microscope
-- End of data disk bogus processes.

-- Gold and bullion production processes were dialed in so the cost of Money is one crono per unit.
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (466,'Gold Ore',159,50,2,45,0); -- Gold Ore
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (466,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (466,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (466,25,1,6); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (466,82,0,6); -- SUV
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (466,114,0,2); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (467,'Gold',160,86,2,30); -- Gold from Gold Ore
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (467,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (467,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (467,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (467,159,20,5); -- Gold Ore
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (468,'Bullion using Gold',157,79,256,5);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (468,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (468,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (468,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (468,160,1,5); -- Gold
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (469,'Bullion using Gold',157,100,256,5);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (469,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (469,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (469,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (469,160,1,5); -- Gold
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (470,'Gold using Bullion',160,86,10,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (470,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (470,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (470,4,0,2); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (470,157,25000,5); -- Cronodollars (gold base price 250 x 100 made)
-- End of bullion production processes.
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (471,'Gills',382,0,1,4,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (471,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (471,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (471,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (471,33,1,5); -- Fish
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (472,'Heart',385,0,1,4,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (472,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (472,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (472,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (472,14,1,5); -- Animal Carcass
-- The next two processes are for event log data disks. The industry is 0 so they can't be manufactured directly.
-- They are here so the cost analyzer can determine their base price.
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (473,'Officer Log Disk using Metal',290,0,20,4,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (473,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (473,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (473,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (473,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (474,'Officer Log Disk using Plastic',290,0,20,4,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (474,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (474,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (474,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (474,63,1,5); -- Plastic
-- End of data disk bogus processes.
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (475,'Wildcatter Bay Module',291,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (475,547,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (475,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (475,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (475,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (475,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (475,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (475,4,0,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (476,'Siphon Bay Module',289,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (476,548,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (476,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (476,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (476,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (476,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (476,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (476,4,0,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (477,'Silage using Beans',295,30,4,5,2);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (477,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (477,15,2,5); -- Beans
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (477,28,0,2); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (477,114,0,2); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (478,'Silage using Fruit',295,30,2,5,2);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (478,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (478,35,2,5); -- Fruit
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (478,28,0,2); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (478,114,0,2); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (479,'Silage using Grain',295,30,3,5,2);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (479,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (479,38,2,5); -- Grain
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (479,28,0,2); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (479,114,0,2); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (480,'Silage using Grapes',295,30,2,5,2);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (480,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (480,39,2,5); -- Grapes
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (480,28,0,2); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (480,114,0,2); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (481,'Silage using Hay',295,30,2,5,2);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (481,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (481,130,2,5); -- Hay
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (481,28,0,2); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (481,114,0,2); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (482,'Silage using Herbs',295,30,2,5,2);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (482,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (482,42,2,5); -- Herbs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (482,28,0,2); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (482,114,0,2); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (483,'Silage using Hops',295,30,2,5,2);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (483,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (483,43,2,5); -- Hops
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (483,28,0,2); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (483,114,0,2); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (484,'Silage using Nuts',295,30,3,5,2);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (484,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (484,58,2,5); -- Nuts
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (484,28,0,2); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (484,114,0,2); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (485,'Silage using Spices',295,30,2,5,2);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (485,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (485,74,2,5); -- Spices
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (485,28,0,2); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (485,114,0,2); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (486,'Silage using Vegetables',295,30,2,5,2);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (486,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (486,81,2,5); -- Vegetables
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (486,28,0,2); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (486,114,0,2); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (487,'Animal Carcass using Silage',14,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (487,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (487,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (487,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (487,295,1,5); -- Silage
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (488,'Cheese using Silage',20,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (488,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (488,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (488,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (488,295,1,5); -- Silage
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (489,'Eggs using Silage',26,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (489,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (489,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (489,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (489,295,1,5); -- Silage
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (490,'Fertilizer using Silage',29,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (490,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (490,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (490,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (490,295,1,5); -- Silage
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (491,'Milk using Silage',52,30,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (491,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (491,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (491,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (491,295,1,5); -- Silage
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (492,'Laser Rifle',296,77,1,255);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (492,549,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (492,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (492,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (492,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (492,37,1,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (492,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (492,63,3,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (492,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (492,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (492,229,0,5); -- Clamp
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (493,'Small Rocket BlBu Firecracker',297,80,1,523);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (493,550,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (493,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (493,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (493,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (493,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (493,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (493,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (493,36,1,5); -- Gasoline
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (494,'Large Rocket BlBu Molotov',298,80,1,798);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (494,551,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (494,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (494,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (494,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (494,64,1,5); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (494,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (494,12,1,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (494,36,1,5); -- Gasoline
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (495,'Small Missile BlBu Barnburner',299,80,1,613);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (495,552,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (495,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (495,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (495,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (495,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (495,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (495,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (495,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (495,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (495,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (495,36,1,5); -- Gasoline
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (496,'Large Missile BlBu Fireball',300,80,1,1582);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (496,553,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (496,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (496,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (496,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (496,27,14,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (496,64,1,5); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (496,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (496,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (496,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (496,12,1,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (496,36,1,5); -- Gasoline
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (497,'Small Rocket SlPi Meathook',301,80,1,523);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (497,554,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (497,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (497,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (497,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (497,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (497,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (497,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (497,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (498,'Large Rocket BuDi Scudpox',302,80,1,798);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (498,555,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (498,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (498,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (498,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (498,64,1,5); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (498,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (498,36,1,5); -- Gasoline
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (498,120,1,5); -- Antitoxin
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (498,12,1,5); -- Bomb
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (499,'Small Missile CrFr Snowball',303,80,1,613);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (499,556,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (499,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (499,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (499,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (499,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (499,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (499,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (499,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (499,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (499,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (499,142,1,5); -- Cryozine
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (500,'Large Missile SlPo Sideviper',304,80,1,1582);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (500,557,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (500,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (500,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (500,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (500,27,14,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (500,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (500,64,1,5); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (500,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (500,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (500,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (500,12,1,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (500,119,1,5); -- Antidote
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (501,'Laser Pistol',305,77,1,192);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (501,558,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (501,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (501,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (501,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (501,37,1,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (501,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (501,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (501,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (501,227,1,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (501,229,0,5); -- Clamp
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (502,'Heavy Arms Ammo PiRa',306,80,1,72);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (502,559,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (502,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (502,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (502,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (502,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (502,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (502,66,1,5); -- Radioactives
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (503,'Heavy Arms Ammo PiBl',307,80,1,72);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (503,560,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (503,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (503,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (503,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (503,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (503,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (503,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (504,'Heavy Arms Ammo PiAc',308,80,1,72);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (504,561,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (504,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (504,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (504,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (504,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (504,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (504,61,1,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (505,'Stasis Beam Lens',309,77,1,250);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (505,562,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (505,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (505,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (505,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (505,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (505,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (505,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (505,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (505,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (505,144,2,5); -- Polytaride
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (505,264,2,5); -- Magmium Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (506,'Neutron Beam Lens',310,77,1,250);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (506,563,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (506,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (506,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (506,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (506,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (506,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (506,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (506,66,2,5); -- Radioactives
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (506,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (506,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (506,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (507,'Electron Beam Lens',311,77,1,250);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (507,564,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (507,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (507,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (507,24,2,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (507,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (507,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (507,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (507,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (507,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (507,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (507,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (508,'Erg Ray Lens',312,77,1,250);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (508,565,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (508,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (508,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (508,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (508,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (508,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (508,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (508,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (508,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (508,228,2,5); -- Io Tube
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (508,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (509,'Freeze Ray Lens',313,77,1,250);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (509,566,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (509,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (509,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (509,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (509,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (509,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (509,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (509,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (509,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (509,142,2,5); -- Cryozine
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (509,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (510,'Compressor Beam Lens',314,77,1,250);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (510,567,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (510,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (510,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (510,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (510,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (510,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (510,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (510,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (510,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (510,265,2,5); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (510,272,2,5); -- Borexino Precipitate
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (511,'Disruptor Beam Lens',315,77,1,250);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (511,568,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (511,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (511,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (511,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (511,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (511,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (511,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (511,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (511,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (511,143,2,5); -- Myrathane
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (511,264,2,5); -- Magmium Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (512,'Heat Ray Lens',316,77,1,250);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (512,569,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (512,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (512,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (512,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (512,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (512,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (512,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (512,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (512,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (512,140,2,5); -- Phlogiston
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (512,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (513,'Proton Beam Lens',317,77,1,250);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (513,570,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (513,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (513,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (513,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (513,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (513,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (513,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (513,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (513,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (513,265,2,5); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (513,271,2,5); -- Antiflux Particles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (514,'Disintegrator Ray Lens',318,77,1,250);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (514,571,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (514,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (514,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (514,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (514,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (514,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (514,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (514,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (514,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (514,141,2,5); -- Bolite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (514,265,2,5); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (515,'Weapon Deployment Bay Module',319,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (515,572,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (515,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (515,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (515,50,3,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (515,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (515,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (515,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (515,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (516,'Bomb BlRa Los Alamos',320,80,1,32); -- Bomb BlRa
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (516,573,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (516,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (516,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (516,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (516,12,2,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (516,66,100,5); -- Radioactives
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (516,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (517,'Bomb BlPi Porcupine',321,80,1,32); -- Bomb BlPi
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (517,574,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (517,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (517,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (517,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (517,12,2,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (517,51,100,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (518,'Bomb ElSt Tesla',322,80,1,32); -- Bomb ElSt
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (518,575,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (518,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (518,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (518,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (518,12,2,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (518,24,25,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (518,51,50,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (518,53,25,5); -- Minerals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (519,'Bomb SuPo Muffler',323,80,1,32); -- Bomb SuPo
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (519,576,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (519,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (519,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (519,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (519,12,2,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (519,61,50,5); -- Petrochemicals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (519,119,50,5); -- Antidote
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (520,'Bomb SlCr Meat Slicer',324,80,1,32); -- Bomb SlCr
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (520,577,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (520,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (520,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (520,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (520,12,2,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (520,51,100,5); -- Metal
-- The next process is for seeds. The industry is 0 so they can't be manufactured directly.
-- This is here so the cost analyzer can determine the base price.
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (521,'Seed',325,0,1,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (521,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (521,28,0,6); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (521,29,1,6); -- Fertilizer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (521,88,0,5); -- Water in Environment
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (521,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (521,114,0,6); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (522,'Small Flower Pot using Plastic',326,82,5,6); -- Small Flower Pot
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (522,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (522,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (522,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (523,'Large Flower Pot using Plastic',327,82,5,8); -- Large Flower Pot
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (523,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (523,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (523,63,2,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (524,'Small Flower Pot using Lumber',326,72,4,8);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (524,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (524,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (524,48,1,5); -- Lumber
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (525,'Large Flower Pot using Lumber',327,72,4,10);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (525,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (525,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (525,48,2,5); -- Lumber
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (526,'Small Flower Pot using Metal',326,79,4,10);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (526,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (526,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (526,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (527,'Large Flower Pot using Metal',327,79,4,12);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (527,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (527,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (527,51,2,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (528,'Genesis Device',328,0,1,1000);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,580,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,3,1,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,51,6,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,63,8,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,141,1,5); -- Bolite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,142,3,5); -- Cryozine
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,144,2,5); -- Polytaride
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,266,21,5); -- Myrasplicer (210 animal species/zone)
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,267,0,5); -- Microscope
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,268,1,5); -- Gigacell
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,270,0,5); -- Nanopath Calibrator
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (528,272,6,5); -- Borexino Precipitate (54 plant species/zone)
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (529,'Picture Frame 60x34cm using Plastic',329,82,8,10); -- Plastic picture frame
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (529,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (529,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (529,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (529,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (530,'Picture Frame 22x28cm using Plastic',330,82,10,10); -- Plastic picture frame
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (530,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (530,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (530,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (530,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (531,'Picture Frame 120x68cm using Plastic',331,82,6,10); -- Plastic picture frame
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (531,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (531,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (531,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (531,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (532,'Candle using Oil',333,72,6,12,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (532,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (532,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (532,23,1,5); -- Oil
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (532,128,0,2); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (533,'Candle using Oil',333,82,6,12,2);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (533,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (533,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (533,23,1,5); -- Oil
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (533,128,0,2); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (534,'Candle using Petrochemicals',333,82,6,12,2); -- Candle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (534,584,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (534,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (534,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (534,61,1,5); -- Petrochemicals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (534,128,0,2); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (535,'Type O Preon Ring',334,78,2,1000);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (535,371,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (535,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (535,3,2,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (535,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (535,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (535,273,2,5); -- Type O Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (536,'Type B Preon Ring',335,78,2,1000);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (536,371,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (536,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (536,3,2,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (536,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (536,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (536,274,2,5); -- Type B Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (537,'Type A Preon Ring',336,78,2,1000);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (537,371,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (537,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (537,3,2,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (537,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (537,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (537,275,2,5); -- Type A Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (538,'Type F Preon Ring',337,78,2,1000);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (538,371,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (538,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (538,3,2,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (538,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (538,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (538,276,2,5); -- Type F Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (539,'Type G Preon Ring',338,78,2,1000);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (539,371,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (539,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (539,3,2,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (539,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (539,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (539,277,2,5); -- Type G Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (540,'Type K Preon Ring',339,78,2,1000);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (540,371,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (540,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (540,3,2,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (540,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (540,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (540,278,2,5); -- Type K Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (541,'Type M Preon Ring',340,78,2,1000);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (541,371,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (541,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (541,3,2,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (541,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (541,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (541,279,2,5); -- Type M Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (542,'ParcX Box using Paper',341,140,25,1,2);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (542,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (542,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (542,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (542,282,1,5); -- Paper
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (543,'Fire using Log',342,0,1,1,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (543,86,1,5); -- Log
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (544,'Tepee using Leather',343,0,1,1,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (544,47,3,5); -- Leather
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (544,86,3,5); -- Log
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (545,'Tepee using Textiles',343,0,1,1,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (545,79,3,5); -- Textile
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (545,86,3,5); -- Log
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (546,'Pressure Tent',344,73,1,500);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (546,374,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (546,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (546,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (546,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (546,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (546,63,3,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (546,79,3,5); -- Textile
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (546,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (546,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (546,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (546,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (547,'Beacon',345,75,2,35);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (547,375,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (547,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (547,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (547,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (547,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (547,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (547,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (548,'Drum',346,0,1,35,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (548,47,1,5); -- Leather
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (548,86,1,5); -- Log
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (549,'Fire using Hay',342,0,1,1,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (549,130,2,5); -- Hay (from shrubbery).
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (550,'Cheese',20,35,1,24,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (550,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (550,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (550,52,1,5); -- Milk
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (551,'Ice Cream',347,35,2,24,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (551,376,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (551,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (551,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (551,52,1,5); -- Milk
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (551,78,1,5); -- Sugar
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (552,'Lungs',404,0,1,4,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (552,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (552,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (552,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (552,14,1,5); -- Animal Carcass
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (553,'Stomach',405,0,1,4,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (553,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (553,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (553,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (553,14,1,5); -- Animal Carcass
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (554,'Fire using Oil',349,0,1,1,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (554,23,1,5); -- Oil
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (555,'Life Support Gravity Module',351,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (555,377,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (555,5,2,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (555,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (555,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (555,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (555,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (555,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (555,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (555,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (555,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (555,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (555,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (555,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (555,226,2,5); -- Grav Coupling
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (555,135,0,5); -- Pliers
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (556,'Red Binder using Paper',412,140,1,10,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (556,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (556,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (556,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (556,282,2,5); -- Paper
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (557,'Red Binder using Plastic',412,82,1,10,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (557,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (557,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (557,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (557,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (558,'Capacitor Module',352,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (558,378,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (558,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (558,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (558,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (558,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (558,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (558,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (559,'Spacecraft Blueprint Disk using Metal',354,0,20,4,0); -- Spacecraft Blueprint Disk
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (559,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (559,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (559,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (559,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (560,'Spacecraft Blueprint Disk using Plastic',354,0,20,4,0); -- Spacecraft Blueprint Disk
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (560,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (560,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (560,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (560,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (561,'Spacecraft Blueprint Drawing using Lumber',353,0,10,10,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (561,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (561,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (561,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (561,48,1,5); -- Lumber

-- The next process is for shipping containers. The industry is 0 so they can't be manufactured directly.
-- This is here so the cost analyzer can determine the base price.
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (565,'TEU Container',355,0,1,4); -- TEU Container
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (565,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (565,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (565,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (565,51,1,5); -- Metal
-- Patent RnD Processes
-- Most patents have two processes each, one for researching at the native industry and the other for researching at a university.
-- An industry cannot be built unless the knowhow to run one of its non-RnD processes is available in town.
-- Therefore, two processes are not needed when defining the only process available to an industry that is not an RnD process.
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (566,'Hydrogen, Method for Refining from Atmosphere',356,83,1,2880,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (566,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (566,44,0,2); -- Hydrogen
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (566,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (566,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (567,'Hydrogen, Method for Refining from Atmosphere',356,125,1,2400,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (567,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (567,44,0,2); -- Hydrogen
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (567,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (567,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (568,'Air, Method for Refining from Atmosphere',357,83,1,2880,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (568,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (568,5,0,2); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (568,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (568,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (569,'Air, Method for Refining from Atmosphere',357,125,1,2400,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (569,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (569,5,0,2); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (569,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (569,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (570,'Helicopter Apparatus',358,70,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (570,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (570,6,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (570,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (570,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (570,50,3,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (570,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (570,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (570,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (570,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (570,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (570,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (571,'Helicopter Apparatus',358,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (571,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (571,6,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (571,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (571,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (571,50,3,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (571,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (571,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (571,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (571,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (571,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (571,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (572,'Space Rocket Apparatus',359,70,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (572,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (572,7,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (572,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (572,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (572,5,1,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (572,50,3,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (572,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (572,64,1,5); -- Large rocket motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (572,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (572,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (572,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (572,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (572,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (573,'Space Rocket Apparatus',359,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (573,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (573,7,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (573,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (573,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (573,5,1,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (573,50,3,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (573,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (573,64,1,5); -- Large rocket motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (573,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (573,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (573,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (573,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (573,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (574,'Space Fighter F1 Apparatus',360,70,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (574,359,0,4); -- Patent: Space Rocket
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (574,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (574,8,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (574,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (574,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (574,5,1,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (574,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (574,50,4,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (574,51,6,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (574,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (574,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (574,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (574,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (574,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (574,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (574,226,1,5); -- Grav Coupling for G-drive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (575,'Space Fighter F1 Apparatus',360,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (575,359,0,4); -- Patent: Space Rocket
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (575,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (575,8,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (575,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (575,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (575,5,1,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (575,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (575,50,4,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (575,51,6,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (575,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (575,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (575,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (575,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (575,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (575,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (575,226,1,5); -- Grav Coupling for G-drive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (576,'Space Transport Apparatus',361,70,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (576,359,0,4); -- Patent: Space Rocket
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (576,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (576,9,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (576,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (576,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (576,5,1,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (576,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (576,50,8,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (576,51,15,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (576,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (576,79,6,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (576,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (576,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (576,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (576,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (576,226,2,5); -- Grav Couplings for G-drive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (577,'Space Transport Apparatus',361,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (577,359,0,4); -- Patent: Space Rocket
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (577,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (577,9,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (577,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (577,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (577,5,1,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (577,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (577,50,8,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (577,51,15,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (577,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (577,79,6,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (577,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (577,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (577,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (577,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (577,226,2,5); -- Grav Couplings for G-drive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (578,'Dynamite, Method for Making',362,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (578,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (578,25,0,2); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (578,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (578,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (579,'Dynamite, Method for Making',362,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (579,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (579,25,0,2); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (579,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (579,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (580,'Heavy Arms Ammo Pi, Method for Making',363,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (580,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (580,11,0,2); -- Heavy Arms Ammo Pi
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (580,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (580,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (580,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (580,51,2,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (581,'Heavy Arms Ammo Pi, Method for Making',363,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (581,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (581,11,0,2); -- Heavy Arms Ammo Pi
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (581,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (581,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (581,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (581,51,2,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (582,'Plastic Explosive, Method for Making',364,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (582,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (582,13,0,2); -- Plastic Explosive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (582,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (582,61,2,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (583,'Plastic Explosive, Method for Making',364,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (583,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (583,13,0,2); -- Plastic Explosive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (583,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (583,61,2,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (584,'Bomb Bl Black Cat, Method for Making using Plastic Explosive',365,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (584,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (584,12,0,2); -- Bomb Bl Black Cat
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (584,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (584,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (584,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (584,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (584,13,100,5); -- Plastic Explosives
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (585,'Bomb Bl Black Cat, Method for Making using Plastic Explosive',365,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (585,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (585,12,0,2); -- Bomb Bl Black Cat
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (585,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (585,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (585,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (585,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (585,13,100,5); -- Plastic Explosives
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (586,'Fertilizer, Method for Refining from Atmosphere',366,83,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (586,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (586,29,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (586,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (586,44,1,5); -- Hydrogen
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (586,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (587,'Fertilizer, Method for Refining from Atmosphere',366,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (587,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (587,29,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (587,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (587,44,1,5); -- Hydrogen
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (587,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (588,'Fertilizer, Method for Refining from Air',367,83,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (588,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (588,29,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (588,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (588,44,1,5); -- Hydrogen
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (588,5,1,5); -- Air
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (589,'Fertilizer, Method for Refining from Air',367,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (589,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (589,29,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (589,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (589,44,1,5); -- Hydrogen
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (589,5,1,5); -- Air
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (590,'Bomb Bl Black Cat, Method for Making using Nitrate Explosive',365,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (590,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (590,12,0,2); -- Bomb Bl Black Cat
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (590,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (590,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (590,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (590,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (590,93,100,5); -- Nitrate Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (591,'Bomb Bl Black Cat, Method for Making using Nitrate Explosive',365,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (591,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (591,12,0,2); -- Bomb Bl Black Cat
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (591,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (591,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (591,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (591,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (591,93,100,5); -- Nitrate Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (592,'Bomb Bl Black Cat, Method for Making using Dynamite',365,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (592,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (592,12,0,2); -- Bomb Bl Black Cat
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (592,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (592,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (592,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (592,25,100,5); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (592,51,4,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (593,'Bomb Bl Black Cat, Method for Making using Dynamite',365,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (593,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (593,12,0,2); -- Bomb Bl Black Cat
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (593,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (593,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (593,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (593,25,100,5); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (593,51,4,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (594,'Large Rocket Motor, Method for Making using Nitrate Explosive',370,84,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (594,440,0,5); -- Patent Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (594,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (594,64,0,2); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (594,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (594,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (594,51,20,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (594,93,40,5); -- Nitrate Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (595,'Large Rocket Motor, Method for Making using Nitrate Explosive',370,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (595,440,0,5); -- Patent Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (595,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (595,64,0,2); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (595,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (595,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (595,51,20,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (595,93,40,5); -- Nitrate Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (596,'Preon Ring, Method for Making using Type G Preon',371,78,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (596,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (596,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (596,338,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (596,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (596,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (596,277,2,5); -- Type G Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (597,'Preon Ring, Method for Making using Type G Preon',371,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (597,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (597,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (597,338,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (597,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (597,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (597,277,2,5); -- Type G Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (598,'Preon Ring, Method for Making using Type K Preon',371,78,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (598,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (598,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (598,339,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (598,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (598,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (598,278,2,5); -- Type K Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (599,'Preon Ring, Method for Making using Type K Preon',371,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (599,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (599,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (599,339,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (599,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (599,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (599,278,2,5); -- Type K Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (600,'Preon Ring, Method for Making using Type M Preon',371,78,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (600,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (600,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (600,340,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (600,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (600,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (600,279,2,5); -- Type M Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (601,'Preon Ring, Method for Making using Type M Preon',371,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (601,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (601,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (601,340,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (601,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (601,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (601,279,2,5); -- Type M Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (602,'Pressure Tent, Method for Making',374,73,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (602,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (602,344,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (602,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (602,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (602,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (602,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (602,63,3,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (602,79,3,5); -- Textile
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (602,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (602,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (602,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (602,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (603,'Pressure Tent, Method for Making',374,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (603,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (603,344,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (603,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (603,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (603,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (603,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (603,63,3,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (603,79,3,5); -- Textile
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (603,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (603,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (603,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (603,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (604,'Beacon, Method for Making',375,75,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (604,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (604,345,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (604,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (604,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (604,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (604,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (604,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (604,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (605,'Beacon, Method for Making',375,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (605,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (605,345,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (605,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (605,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (605,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (605,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (605,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (605,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (606,'Ice Cream, Method for Making',376,35,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (606,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (606,347,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (606,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (606,52,1,5); -- Milk
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (606,78,1,5); -- Sugar
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (607,'Ice Cream, Method for Making',376,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (607,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (607,347,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (607,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (607,52,1,5); -- Milk
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (607,78,1,5); -- Sugar
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (608,'Life Support Gravity Module, Method for Making',377,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (608,512,0,5); -- Patent Life Support Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (608,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (608,351,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (608,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (608,5,2,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (608,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (608,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (608,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (608,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (608,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (608,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (608,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (608,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (608,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (608,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (608,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (608,226,2,5); -- Grav Coupling
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (608,135,0,5); -- Pliers
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (609,'Life Support Gravity Module, Method for Making',377,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (609,512,0,5); -- Patent Life Support Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (609,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (609,351,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (609,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (609,5,2,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (609,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (609,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (609,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (609,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (609,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (609,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (609,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (609,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (609,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (609,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (609,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (609,226,2,5); -- Grav Coupling
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (609,135,0,5); -- Pliers
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (610,'Capacitor Module, Method for Making',378,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (610,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (610,352,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (610,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (610,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (610,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (610,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (610,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (610,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (610,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (611,'Capacitor Module, Method for Making',378,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (611,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (611,352,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (611,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (611,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (611,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (611,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (611,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (611,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (611,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (615,'Candy, Method for Making',380,40,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (615,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (615,19,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (615,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (615,52,1,5); -- Milk
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (615,74,1,5); -- Spices
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (615,78,1,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (615,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (616,'Candy, Method for Making',380,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (616,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (616,19,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (616,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (616,52,1,5); -- Milk
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (616,74,1,5); -- Spices
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (616,78,1,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (616,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (617,'Hydrogen, Method for Making from Natural Gas',381,83,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (617,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (617,44,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (617,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (617,57,1,5); -- Natural Gas
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (618,'Hydrogen, Method for Making from Natural Gas',381,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (618,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (618,44,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (618,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (618,57,1,5); -- Natural Gas
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (619,'Large Rocket Motor, Method for Making using Gun Powder',370,84,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (619,440,0,5); -- Patent Small Rocket Motor using Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (619,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (619,64,0,2); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (619,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (619,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (619,40,40,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (619,51,20,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (620,'Large Rocket Motor, Method for Making using Gun Powder',370,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (620,440,0,5); -- Patent: Small Rocket Motor using Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (620,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (620,64,0,2); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (620,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (620,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (620,40,40,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (620,51,20,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (621,'Computer, Method for Making',383,74,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (621,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (621,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (621,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (621,27,5,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (621,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (621,63,4,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (621,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (621,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (622,'Computer, Method for Making',383,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (622,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (622,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (622,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (622,27,5,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (622,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (622,63,4,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (622,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (622,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (623,'Motor Bike Apparatus',384,88,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (623,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (623,133,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (623,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (623,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (623,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (623,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (623,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (623,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (623,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (623,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (624,'Motor Bike Apparatus',384,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (624,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (624,133,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (624,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (624,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (624,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (624,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (624,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (624,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (624,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (624,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (625,'Large Rocket Motor, Method for Making using Petrochemical',370,84,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (625,440,0,5); -- Patent: Small Rocket Motor using Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (625,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (625,64,0,2); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (625,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (625,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (625,51,20,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (625,61,40,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (626,'Large Rocket Motor, Method for Making using Petrochemical',370,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (626,440,0,5); -- Patent: Small Rocket Motor using Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (626,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (626,64,0,2); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (626,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (626,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (626,51,20,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (626,61,40,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (627,'Rifle Ammunition, Method for Making',386,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (627,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (627,75,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (627,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (627,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (627,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (627,51,2,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (628,'Rifle Ammunition, Method for Making',386,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (628,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (628,75,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (628,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (628,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (628,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (628,51,2,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (629,'Whiskey, Method for Distilling',387,42,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (629,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (629,84,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (629,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (629,38,1,5); -- Grain
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (629,78,1,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (629,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (630,'Whiskey, Method for Distilling',387,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (630,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (630,84,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (630,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (630,38,1,5); -- Grain
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (630,78,1,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (630,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (633,'Pistol Ammunition, Method for Making',389,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (633,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (633,10,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (633,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (633,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (633,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (633,51,2,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (634,'Pistol Ammunition, Method for Making',389,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (634,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (634,10,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (634,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (634,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (634,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (634,51,2,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (635,'Gun Powder, Method for Making',390,76,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (635,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (635,40,0,2); --
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (635,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (635,22,1,5); -- Coal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (635,53,1,5); -- Minerals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (635,93,1,5); -- Nitrate Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (636,'Gun Powder, Method for Making',390,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (636,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (636,40,0,2); --
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (636,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (636,22,1,5); -- Coal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (636,53,1,5); -- Minerals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (636,93,1,5); -- Nitrate Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (638,'Electricity, Method for Generating using Coal',391,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (638,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (638,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (638,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (638,22,1,5); -- Coal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (638,114,0,5); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (640,'Electricity, Method for Generating using Gasoline',392,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (640,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (640,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (640,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (640,36,1,5); -- Gasoline
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (640,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (642,'Electricity, Method for Generating using Hydrogen',393,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (642,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (642,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (642,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (642,44,1,5); -- Hydrogen
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (642,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (644,'Electricity, Method for Generating using Natural Gas',394,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (644,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (644,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (644,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (644,57,1,5); -- Natural Gas
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (644,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (646,'Electricity, Method for Generating using Sunlight',395,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (646,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (646,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (646,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (646,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (648,'Electricity, Method for Generating using Wind',396,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (648,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (648,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (648,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (648,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (650,'Electricity, Method for Generating using Radioactives',397,0,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (650,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (650,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (650,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (650,66,1,5); -- Radioactives
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (650,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (651,'Electronic Part, Method for Making',398,75,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (651,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (651,27,0,2); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (651,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (651,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (651,24,1,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (651,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (651,53,1,5); -- Minerals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (651,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (651,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (652,'Electronic Part, Method for Making',398,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (652,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (652,27,0,2); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (652,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (652,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (652,24,1,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (652,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (652,53,1,5); -- Minerals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (652,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (652,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (653,'Farm Machinery Apparatus',399,88,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (653,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (653,28,0,2); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (653,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (653,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (653,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (653,50,4,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (653,51,8,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (653,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (654,'Farm Machinery Apparatus',399,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (654,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (654,28,0,2); -- Farm Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (654,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (654,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (654,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (654,50,4,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (654,51,8,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (654,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (655,'Knife, Method for Making using Metal and Plastic',400,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (655,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (655,116,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (655,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (655,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (655,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (655,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (656,'Knife, Method for Making using Metal and Plastic',400,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (656,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (656,116,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (656,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (656,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (656,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (656,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (657,'Water, Method for Making using Air and Hydrogen',401,83,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (657,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (657,83,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (657,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (657,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (657,5,1,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (657,44,1,5); -- Hydrogen
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (658,'Water, Method for Making using Air and Hydrogen',401,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (658,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (658,83,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (658,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (658,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (658,5,1,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (658,44,1,5); -- Hydrogen
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (659,'Pistol, Method for Making using Metal and Wood',402,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (659,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (659,31,0,2); -- Pistol
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (659,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (659,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (659,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (659,86,2,5); -- Wood
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (660,'Pistol, Method for Making using Metal and Wood',402,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (660,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (660,31,0,2); -- Pistol
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (660,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (660,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (660,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (660,86,2,5); -- Wood
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (661,'Vodka, Method for Distilling',403,42,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (661,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (661,101,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (661,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (661,38,1,5); -- Grain
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (661,78,1,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (661,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (662,'Vodka, Method for Distilling',403,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (662,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (662,101,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (662,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (662,38,1,5); -- Grain
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (662,78,1,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (662,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (663,'Pistol, Method for Making using Metal',402,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (663,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (663,31,0,2); -- Pistol
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (663,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (663,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (663,51,3,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (664,'Pistol, Method for Making using Metal',402,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (664,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (664,31,0,2); -- Pistol
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (664,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (664,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (664,51,3,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (665,'Pistol, Method for Making using Metal and Plastic',402,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (665,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (665,31,0,2); -- Pistol
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (665,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (665,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (665,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (665,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (666,'Pistol, Method for Making using Metal and Plastic',402,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (666,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (666,31,0,2); -- Pistol
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (666,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (666,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (666,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (666,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (667,'Rifle, Method for Making using Metal',406,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (667,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (667,32,0,2); -- Rifle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (667,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (667,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (667,51,4,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (668,'Rifle, Method for Making using Metal',406,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (668,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (668,32,0,2); -- Rifle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (668,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (668,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (668,51,4,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (669,'Кала́шников, Method for Making using Metal',407,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (669,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (669,292,0,2); -- AK
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (669,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (669,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (669,51,4,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (670,'Кала́шников, Method for Making using Metal',407,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (670,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (670,292,0,2); -- AK
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (670,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (670,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (670,51,4,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (671,'Rifle, Method for Making using Metal and Plastic',406,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (671,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (671,32,0,2); -- Rifle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (671,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (671,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (671,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (671,63,2,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (672,'Rifle, Method for Making using Metal and Plastic',406,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (672,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (672,32,0,2); -- Rifle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (672,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (672,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (672,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (672,63,2,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (673,'Rifle, Method for Making using Metal and Wood',406,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (673,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (673,32,0,2); -- Rifle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (673,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (673,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (673,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (673,86,2,5); -- Wood
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (674,'Rifle, Method for Making using Metal and Wood',406,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (674,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (674,32,0,2); -- Rifle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (674,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (674,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (674,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (674,86,2,5); -- Wood
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (675,'Кала́шников, Method for Making using Metal and Plastic',407,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (675,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (675,292,0,2); -- AK
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (675,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (675,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (675,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (675,63,2,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (676,'Кала́шников, Method for Making using Metal and Plastic',407,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (676,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (676,292,0,2); -- AK
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (676,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (676,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (676,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (676,63,2,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (677,'Gasoline, Method for Refining',411,83,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (677,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (677,36,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (677,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (677,23,1,5); -- Oil
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (678,'Gasoline, Method for Refining',411,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (678,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (678,36,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (678,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (678,23,1,5); -- Oil
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (679,'Кала́шников, Method for Making using Metal and Wood',407,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (679,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (679,292,0,2); -- AK
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (679,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (679,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (679,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (679,86,2,5); -- Wood
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (680,'Кала́шников, Method for Making using Metal and Wood',407,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (680,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (680,292,0,2); -- AK
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (680,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (680,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (680,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (680,86,2,5); -- Wood
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (681,'Electricity, Method for Generating using Water in Environment',413,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (681,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (681,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (681,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (681,88,0,5); -- Water in Environment needed but not consumed
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (682,'Nitrate Explosive, Method for Making',414,76,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (682,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (682,93,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (682,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (682,29,4,5); -- Fertilizer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (683,'Nitrate Explosive, Method for Making',414,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (683,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (683,93,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (683,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (683,29,4,5); -- Fertilizer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (686,'Petrochemicals, Method for Refining',416,83,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (686,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (686,61,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (686,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (686,23,1,5); -- Oil
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (687,'Petrochemicals, Method for Refining',416,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (687,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (687,61,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (687,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (687,23,1,5); -- Oil
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (688,'Hydrogen, Method for Refining from Water',417,83,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (688,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (688,44,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (688,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (688,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (688,83,2,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (689,'Hydrogen, Method for Refining from Water',417,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (689,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (689,44,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (689,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (689,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (689,83,2,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (690,'Grav Coupling, Method for Making',418,75,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (690,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (690,226,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (690,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (690,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (690,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (690,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (690,134,2,5); -- Eludium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (691,'Grav Coupling, Method for Making',418,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (691,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (691,226,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (691,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (691,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (691,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (691,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (691,134,2,5); -- Eludium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (692,'Cryo Heat Sink, Method for Making',419,79,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (692,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (692,227,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (692,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (692,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (692,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (692,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (692,142,2,5); -- Cryozine
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (693,'Cryo Heat Sink, Method for Making',419,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (693,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (693,227,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (693,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (693,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (693,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (693,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (693,142,2,5); -- Cryozine
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (694,'Magmium Knife, Method for Making using Magmium and Wood',420,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (694,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (694,151,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (694,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (694,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (694,86,1,5); -- Wood
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (694,146,1,5); -- Magmium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (695,'Magmium Knife, Method for Making using Magmium and Wood',420,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (695,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (695,151,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (695,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (695,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (695,86,1,5); -- Wood
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (695,146,1,5); -- Magmium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (696,'Magmium Knife, Method for Making using Magmium',420,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (696,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (696,151,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (696,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (696,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (696,146,2,5); -- Magmium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (697,'Magmium Knife, Method for Making using Magmium',420,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (697,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (697,151,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (697,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (697,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (697,146,2,5); -- Magmium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (698,'Magmium Knife, Method for Making using Magmium and Plastic',420,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (698,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (698,151,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (698,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (698,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (698,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (698,146,1,5); -- Magmium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (699,'Magmium Knife, Method for Making using Magmium and Plastic',420,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (699,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (699,151,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (699,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (699,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (699,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (699,146,1,5); -- Magmium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (700,'Io Tube, Method for Making',423,75,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (700,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (700,228,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (700,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (700,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (700,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (700,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (700,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (700,138,2,5); -- Ioplasma
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (701,'Io Tube, Method for Making',423,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (701,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (701,228,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (701,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (701,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (701,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (701,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (701,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (701,138,2,5); -- Ioplasma
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (702,'Plastic, Method for Making',424,82,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (702,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (702,63,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (702,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (702,61,2,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (703,'Plastic, Method for Making',424,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (703,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (703,63,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (703,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (703,61,2,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (704,'Mine Machinery Apparatus',425,0,1,2880,0); -- disabled
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (704,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (704,54,0,2); -- Mine Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (704,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (704,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (704,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (704,50,6,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (704,51,10,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (704,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (705,'Mine Machinery Apparatus',425,0,1,2400,0); -- disabled
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (705,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (705,54,0,2); -- Mine Machinery
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (705,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (705,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (705,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (705,50,6,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (705,51,10,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (705,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (706,'Small Missile Bl Zipper, Method for Making using Dynamite',426,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (706,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (706,55,0,2); -- Small Missile Bl Zipper
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (706,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (706,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (706,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (706,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (706,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (706,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (706,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (706,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (706,25,1,5); -- Dynamite
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (707,'Small Missile Bl Zipper, Method for Making using Dynamite',426,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (707,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (707,55,0,2); -- Small Missile Bl Zipper
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (707,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (707,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (707,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (707,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (707,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (707,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (707,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (707,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (707,25,1,5); -- Dynamite
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (708,'Small Missile Bl Zipper, Method for Making using Plastic Explosive',426,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (708,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (708,55,0,2); -- Small Missile Bl Zipper
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (708,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (708,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (708,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (708,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (708,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (708,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (708,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (708,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (708,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (709,'Small Missile Bl Zipper, Method for Making using Plastic Explosive',426,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (709,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (709,55,0,2); -- Small Missile Bl Zipper
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (709,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (709,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (709,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (709,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (709,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (709,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (709,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (709,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (709,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (710,'Small Missile Bl Zipper, Method for Making using Nitrate Explosive',426,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (710,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (710,55,0,2); -- Small Missile Bl Zipper
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (710,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (710,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (710,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (710,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (710,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (710,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (710,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (710,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (710,93,1,5); -- Nitrate Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (711,'Small Missile Bl Zipper, Method for Making using Nitrate Explosive',426,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (711,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (711,55,0,2); -- Small Missile Bl Zipper
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (711,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (711,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (711,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (711,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (711,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (711,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (711,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (711,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (711,93,1,5); -- Nitrate Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (712,'Large Missile Bl Thumper, Method for Making',429,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (712,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (712,56,0,2); -- Large Missile Bl Thumper
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (712,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (712,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (712,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (712,27,14,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (712,64,1,5); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (712,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (712,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (712,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (712,12,1,5); -- Bomb
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (713,'Large Missile Bl Thumper, Method for Making',429,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (713,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (713,56,0,2); -- Large Missile Bl Thumper
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (713,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (713,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (713,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (713,27,14,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (713,64,1,5); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (713,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (713,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (713,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (713,12,1,5); -- Bomb
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (714,'Pastry, Method for Making',430,40,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (714,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (714,60,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (714,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (714,26,1,5); -- Eggs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (714,34,1,5); -- Flour
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (714,35,1,5); -- Fruit
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (714,52,1,5); -- Milk
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (714,58,1,5); -- Nuts
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (714,74,1,5); -- Spices
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (714,78,1,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (714,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (715,'Pastry, Method for Making',430,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (715,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (715,60,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (715,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (715,26,1,5); -- Eggs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (715,34,1,5); -- Flour
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (715,35,1,5); -- Fruit
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (715,52,1,5); -- Milk
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (715,58,1,5); -- Nuts
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (715,74,1,5); -- Spices
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (715,78,1,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (715,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (716,'Air, Method for Refining from Water',431,83,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (716,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (716,5,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (716,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (716,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (716,83,2,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (717,'Air, Method for Refining from Water',431,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (717,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (717,5,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (717,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (717,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (717,83,2,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (718,'Pharmaceuticals, Method for Making using Petrochemicals',432,81,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (718,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (718,62,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (718,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (718,61,1,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (719,'Pharmaceuticals, Method for Making using Petrochemicals',432,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (719,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (719,62,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (719,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (719,61,1,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (722,'Metal EV Suit, Method for Making',434,71,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (722,452,0,5); -- Patent: EV Suit
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (722,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (722,65,0,2); -- Metal EV Suit
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (722,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (722,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (722,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (722,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (722,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (722,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (722,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (722,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (722,50,1,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (722,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (722,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (722,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (722,227,1,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (723,'Metal EV Suit, Method for Making',434,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (723,452,0,5); -- Patent: EV Suit
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (723,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (723,65,0,2); -- Metal EV Suit
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (723,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (723,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (723,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (723,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (723,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (723,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (723,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (723,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (723,50,1,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (723,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (723,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (723,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (723,227,1,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (724,'Radioactives, Method for Mining',435,50,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (724,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (724,66,1,5); -- Radioactives
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (724,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (724,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (725,'Radioactives, Method for Mining',435,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (725,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (725,66,1,5); -- Radioactives
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (725,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (725,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (726,'Small Rocket Bl Partypopper, Method for Making using Dynamite',436,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (726,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (726,67,0,2); -- Small Rocket Bl Partypopper
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (726,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (726,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (726,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (726,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (726,25,1,5); -- Dynamite
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (727,'Small Rocket Bl Partypopper, Method for Making using Dynamite',436,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (727,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (727,67,0,2); -- Small Rocket Bl Partypopper
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (727,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (727,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (727,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (727,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (727,25,1,5); -- Dynamite
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (728,'Small Rocket Bl Partypopper, Method for Making using Plastic Explosive',436,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (728,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (728,67,0,2); -- Small Rocket Bl Partypopper
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (728,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (728,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (728,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (728,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (728,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (729,'Small Rocket Bl Partypopper, Method for Making using Plastic Explosive',436,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (729,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (729,67,0,2); -- Small Rocket Bl Partypopper
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (729,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (729,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (729,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (729,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (729,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (730,'Small Rocket Bl Partypopper, Method for Making using Nitrate Explosive',436,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (730,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (730,67,0,2); -- Small Rocket Bl Partypopper
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (730,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (730,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (730,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (730,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (730,93,1,5); -- Nitrate Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (731,'Small Rocket Bl Partypopper, Method for Making using Nitrate Explosive',436,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (731,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (731,67,0,2); -- Small Rocket Bl Partypopper
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (731,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (731,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (731,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (731,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (731,93,1,5); -- Nitrate Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (732,'Large Rocket Bl Thud, Method for Making',439,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (732,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (732,68,0,2); -- Large Rocket Bl Thud
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (732,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (732,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (732,64,1,5); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (732,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (732,12,1,5); -- Bomb
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (733,'Large Rocket Bl Thud, Method for Making',439,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (733,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (733,68,0,2); -- Large Rocket Bl Thud
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (733,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (733,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (733,64,1,5); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (733,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (733,12,1,5); -- Bomb
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (734,'Small Rocket Motor, Method for Making using Gun Powder',440,84,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (734,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (734,69,0,2); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (734,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (734,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (734,40,10,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (734,51,10,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (735,'Small Rocket Motor, Method for Making using Gun Powder',440,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (735,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (735,69,0,2); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (735,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (735,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (735,40,10,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (735,51,10,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (736,'Small Rocket Motor, Method for Making using Petrochemicals',440,84,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (736,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (736,69,0,2); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (736,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (736,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (736,51,10,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (736,61,10,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (737,'Small Rocket Motor, Method for Making using Petrochemicals',440,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (737,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (737,69,0,2); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (737,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (737,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (737,51,10,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (737,61,10,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (738,'Plastic Sailboat Apparatus',442,85,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (738,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (738,132,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (738,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (738,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (738,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (738,63,7,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (738,79,3,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (739,'Plastic Sailboat Apparatus',442,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (739,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (739,132,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (739,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (739,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (739,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (739,63,7,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (739,79,3,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (740,'Tank Apparatus',443,0,1,2880,0); -- disabled
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (740,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (740,103,0,2); -- Tank
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (740,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (740,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (740,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (740,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (740,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (740,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (740,50,4,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (740,51,10,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (740,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (740,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (741,'Tank Apparatus',443,0,1,2400,0); -- disabled
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (741,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (741,103,0,2); -- Tank
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (741,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (741,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (741,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (741,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (741,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (741,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (741,50,4,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (741,51,10,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (741,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (741,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (742,'APC Apparatus',444,88,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (742,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (742,104,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (742,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (742,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (742,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (742,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (742,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (742,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (742,50,3,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (742,51,8,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (742,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (742,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (743,'APC Apparatus',444,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (743,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (743,104,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (743,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (743,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (743,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (743,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (743,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (743,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (743,50,3,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (743,51,8,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (743,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (743,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (744,'Metal EV Helmet, Method for Making',445,71,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (744,453,0,5); -- Patent: EV Helmet
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (744,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (744,106,0,2); -- Metal EV Helmet
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (744,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (744,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (744,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (744,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (744,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (744,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (744,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (744,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (744,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (744,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (744,79,1,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (745,'Metal EV Helmet, Method for Making',445,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (745,453,0,5); -- Patent: EV Helmet
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (745,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (745,106,0,2); -- Metal EV Helmet
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (745,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (745,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (745,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (745,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (745,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (745,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (745,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (745,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (745,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (745,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (745,79,1,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (746,'Magmium EV Helmet, Method for Making',446,71,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (746,445,0,5); -- Patent: Metal EV Helmet
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (746,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (746,154,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (746,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (746,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (746,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (746,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (746,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (746,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (746,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (746,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (746,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (746,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (746,146,1,5); -- Magmium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (747,'Magmium EV Helmet, Method for Making',446,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (747,445,0,5); -- Patent: Metal EV Helmet
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (747,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (747,154,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (747,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (747,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (747,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (747,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (747,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (747,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (747,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (747,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (747,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (747,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (747,146,1,5); -- Magmium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (748,'Commercial Ship Apparatus',447,0,1,2880,0); -- disabled
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (748,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (748,70,0,2); -- Commercial Ship
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (748,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (748,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (748,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (748,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (748,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (748,27,3,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (748,48,25,5); -- Lumber
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (748,50,9,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (748,51,30,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (748,63,15,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (748,79,22,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (748,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (749,'Commercial Ship Apparatus',447,0,1,2400,0); -- disabled
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (749,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (749,70,0,2); -- Commercial Ship
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (749,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (749,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (749,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (749,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (749,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (749,27,3,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (749,48,25,5); -- Lumber
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (749,50,9,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (749,51,30,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (749,63,15,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (749,79,22,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (749,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (750,'Battle Ship Apparatus',448,0,1,2880,0); -- disabled
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (750,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (750,71,0,2); -- Battle Ship
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (750,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (750,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (750,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (750,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (750,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (750,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (750,48,20,5); -- Lumber
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (750,50,16,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (750,51,45,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (750,63,12,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (750,79,16,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (750,102,2,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (750,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (751,'Battle Ship Apparatus',448,0,1,2400,0); -- disabled
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (751,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (751,71,0,2); -- Battle Ship
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (751,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (751,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (751,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (751,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (751,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (751,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (751,48,20,5); -- Lumber
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (751,50,16,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (751,51,45,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (751,63,12,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (751,79,16,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (751,102,2,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (751,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (752,'Submarine Apparatus',449,85,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (752,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (752,77,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (752,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (752,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (752,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (752,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (752,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (752,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (752,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (752,50,7,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (752,51,9,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (752,63,9,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (752,79,4,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (753,'Submarine Apparatus',449,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (753,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (753,77,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (753,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (753,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (753,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (753,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (753,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (753,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (753,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (753,50,7,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (753,51,9,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (753,63,9,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (753,79,4,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (754,'Textiles, Method for Weaving using Plastic',450,47,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (754,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (754,79,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (754,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (754,63,2,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (755,'Textiles, Method for Weaving using Plastic',450,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (755,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (755,79,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (755,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (755,63,2,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (756,'Antiflux Particles, Method for Refining',113,83,1,2880,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (756,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (756,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (756,271,0,2); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (756,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (756,269,0,5); -- Tesloid Dissipator
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (757,'Antiflux Particles, Method for Refining',113,125,1,2400,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (757,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (757,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (757,271,0,2); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (757,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (757,269,0,5); -- Tesloid Dissipator
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (758,'EV Suit, Method for Making',452,71,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (758,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (758,80,0,2); -- EV Suit
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (758,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (758,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (758,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (758,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (758,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (758,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (758,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (758,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (758,79,1,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (759,'EV Suit, Method for Making',452,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (759,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (759,80,0,2); -- EV Suit
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (759,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (759,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (759,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (759,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (759,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (759,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (759,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (759,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (759,79,1,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (760,'EV Helmet, Method for Making',453,71,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (760,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (760,72,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (760,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (760,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (760,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (760,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (760,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (760,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (760,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (760,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (760,79,1,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (761,'EV Helmet, Method for Making',453,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (761,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (761,72,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (761,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (761,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (761,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (761,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (761,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (761,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (761,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (761,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (761,79,1,5); -- Textiles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (762,'SUV Apparatus',454,88,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (762,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (762,82,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (762,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (762,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (762,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (762,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (762,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (762,50,4,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (762,51,5,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (762,63,4,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (762,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (762,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (763,'SUV Apparatus',454,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (763,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (763,82,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (763,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (763,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (763,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (763,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (763,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (763,50,4,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (763,51,5,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (763,63,4,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (763,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (763,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (764,'Gin, Method for Distilling',455,42,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (764,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (764,115,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (764,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (764,38,1,5); -- Grain
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (764,74,1,5); -- Spices
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (764,78,1,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (764,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (765,'Gin, Method for Distilling',455,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (765,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (765,115,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (765,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (765,38,1,5); -- Grain
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (765,74,1,5); -- Spices
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (765,78,1,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (765,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (766,'Heavy Weapon, Method for Making',456,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (766,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (766,102,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (766,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (766,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (766,50,1,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (766,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (766,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (766,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (767,'Heavy Weapon, Method for Making',456,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (767,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (767,102,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (767,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (767,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (767,50,1,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (767,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (767,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (767,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (768,'Rum, Method for Distilling',457,42,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (768,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (768,121,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (768,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (768,74,1,5); -- Spices
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (768,78,2,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (768,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (769,'Rum, Method for Distilling',457,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (769,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (769,121,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (769,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (769,74,1,5); -- Spices
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (769,78,2,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (769,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (770,'Small Rocket Motor, Method for Making using Nitrate Explosive',440,84,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (770,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (770,69,0,2); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (770,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (770,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (770,51,10,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (770,93,10,5); -- Nitrate Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (771,'Small Rocket Motor, Method for Making using Nitrate Explosive',440,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (771,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (771,69,0,2); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (771,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (771,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (771,51,10,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (771,93,10,5); -- Nitrate Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (772,'Tequila, Method for Distilling',459,42,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (772,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (772,122,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (772,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (772,42,3,5); -- Herbs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (772,78,1,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (772,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (773,'Tequila, Method for Distilling',459,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (773,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (773,122,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (773,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (773,42,3,5); -- Herbs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (773,78,1,5); -- Sugar
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (773,83,1,5); -- Water
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (774,'Borexino Precipitate, Method for Refining from Atmosphere',158,83,1,2880,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (774,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (774,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (774,272,0,2); -- Borexino Precipitate
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (774,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (774,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (774,269,0,5); -- Tesloid Dissipator
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (775,'Borexino Precipitate, Method for Refining from Atmosphere',158,125,1,2400,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (775,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (775,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (775,272,0,2); -- Borexino Precipitate
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (775,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (775,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (775,269,0,5); -- Tesloid Dissipator
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (776,'Lantern, Method for Making using Electricity',461,75,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (776,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (776,108,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (776,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (776,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (776,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (776,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (776,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (777,'Lantern, Method for Making using Electricity',461,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (777,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (777,108,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (777,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (777,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (777,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (777,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (777,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (778,'Lumenite, Method for Mining',462,50,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (778,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (778,111,1,5); -- Lumenite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (778,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (778,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (779,'Lumenite, Method for Mining',462,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (779,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (779,111,1,5); -- Lumenite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (779,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (779,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (780,'Blank Disk, Method for Making using Metal',463,75,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (780,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (780,112,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (780,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (780,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (780,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (781,'Blank Disk, Method for Making using Metal',463,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (781,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (781,112,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (781,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (781,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (781,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (782,'Blank Disk, Method for Making using Plastic',463,75,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (782,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (782,112,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (782,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (782,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (782,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (783,'Blank Disk, Method for Making using Plastic',463,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (783,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (783,112,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (783,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (783,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (783,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (784,'Antidote, Method for Making using Herbs',465,81,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (784,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (784,119,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (784,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (784,42,1,5); -- Herbs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (784,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (785,'Antidote, Method for Making using Herbs',465,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (785,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (785,119,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (785,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (785,42,1,5); -- Herbs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (785,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (786,'Antidote, Method for Making using Minerals',466,81,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (786,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (785,119,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (786,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (786,53,1,5); -- Minerals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (786,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (787,'Antidote, Method for Making using Minerals',466,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (787,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (787,119,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (787,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (787,53,1,5); -- Minerals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (787,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (788,'Antidote, Method for Making using Petrochemicals',467,81,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (788,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (788,119,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (788,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (788,61,1,5); -- Petrochemicals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (788,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (789,'Antidote, Method for Making using Petrochemicals',467,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (789,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (789,119,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (789,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (789,61,1,5); -- Petrochemicals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (789,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (790,'Antitoxin, Method for Making using Animal Carcass',468,81,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (790,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (790,120,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (790,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (790,14,2,5); -- Animals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (790,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (791,'Antitoxin, Method for Making using Animal Carcass',468,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (791,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (791,120,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (791,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (791,14,2,5); -- Animals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (791,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (792,'Antitoxin, Method for Making using Eggs',469,81,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (792,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (792,120,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (792,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (792,26,4,5); -- Eggs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (792,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (793,'Antitoxin, Method for Making using Eggs',469,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (793,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (793,120,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (793,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (793,26,4,5); -- Eggs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (793,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (794,'Shotgun, Method for Making using Metal',470,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (794,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (794,117,0,2); -- Shotgun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (794,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (794,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (794,51,4,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (795,'Shotgun, Method for Making using Metal',470,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (795,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (795,117,0,2); -- Shotgun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (795,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (795,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (795,51,4,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (796,'Shotgun, Method for Making using Metal and Plastic',470,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (796,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (796,117,0,2); -- Shotgun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (796,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (796,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (796,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (796,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (797,'Shotgun, Method for Making using Metal and Plastic',470,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (797,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (797,117,0,2); -- Shotgun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (797,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (797,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (797,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (797,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (798,'Shotgun, Method for Making using Metal and Wood',470,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (798,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (798,117,0,2); -- Shotgun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (798,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (798,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (798,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (798,86,1,5); -- Wood
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (799,'Shotgun, Method for Making using Metal and Wood',470,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (799,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (799,117,0,2); -- Shotgun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (799,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (799,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (799,51,3,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (799,86,1,5); -- Wood
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (800,'Eludium, Method for Mining',473,50,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (800,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (800,134,1,5); -- Eludium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (800,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (800,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (801,'Eludium, Method for Mining',473,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (801,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (801,134,1,5); -- Eludium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (801,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (801,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (802,'Shotgun Ammunition, Method for Making',474,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (802,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (802,118,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (802,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (802,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (802,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (802,51,2,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (803,'Shotgun Ammunition, Method for Making',474,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (803,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (803,118,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (803,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (803,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (803,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (803,51,2,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (806,'Glue Gun, Method for Making using Electricity',476,75,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (806,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (806,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (806,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (806,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (806,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (806,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (807,'Glue Gun, Method for Making using Electricity',476,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (807,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (807,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (807,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (807,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (807,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (807,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (808,'Glue Gun, Method for Making using Natural Gas',476,79,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (808,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (808,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (808,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (808,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (808,57,1,5); -- Natural Gas
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (809,'Glue Gun, Method for Making using Natural Gas',476,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (809,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (809,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (809,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (809,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (809,57,1,5); -- Natural Gas
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (810,'Soldering Iron, Method for Making using Electricity',478,75,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (810,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (810,127,0,2); -- Soldering Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (810,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (810,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (810,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (810,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (811,'Soldering Iron, Method for Making using Electricity',478,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (811,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (811,127,0,2); -- Soldering Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (811,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (811,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (811,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (811,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (812,'Soldering Iron, Method for Making using Natural Gas',478,79,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (812,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (812,127,0,2); -- Soldering Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (812,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (812,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (812,57,1,5); -- Natural Gas
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (813,'Soldering Iron, Method for Making using Natural Gas',478,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (813,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (813,127,0,2); -- Soldering Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (813,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (813,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (813,57,1,5); -- Natural Gas
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (824,'Grav Pack, Method for Making using Leather',485,73,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (824,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (824,294,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (824,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (824,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (824,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (824,135,0,5); -- Pliers
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (824,47,2,5); -- Leather
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (824,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (824,226,1,5); -- Grav Coupling
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (825,'Grav Pack, Method for Making using Leather',485,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (825,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (825,294,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (825,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (825,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (825,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (825,135,0,5); -- Pliers
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (825,47,2,5); -- Leather
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (825,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (825,226,1,5); -- Grav Coupling
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (826,'Grav Pack, Method for Making using Textiles',485,73,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (826,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (826,294,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (826,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (826,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (826,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (826,135,0,5); -- Pliers
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (826,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (826,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (826,226,1,5); -- Grav Coupling
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (827,'Grav Pack, Method for Making using Textiles',485,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (827,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (827,294,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (827,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (827,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (827,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (827,135,0,5); -- Pliers
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (827,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (827,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (827,226,1,5); -- Grav Coupling
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (833,'Vulcanite, Method for Mining',489,50,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (833,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (833,137,1,5); -- Vulcanite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (833,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (833,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (833,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (834,'Vulcanite, Method for Mining',489,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (834,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (834,137,1,5); -- Vulcanite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (834,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (834,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (834,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (835,'Bolite, Method for Mining',490,50,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (835,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (835,141,1,5); -- Bolite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (835,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (835,114,0,5); -- Shovel
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (835,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (836,'Bolite, Method for Mining',490,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (836,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (836,141,1,5); -- Bolite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (836,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (836,114,0,5); -- Shovel
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (836,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (837,'Ioplasma, Method for Refining',491,83,1,2880,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (837,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (837,138,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (837,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (837,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (837,79,0,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (837,149,0,5); -- Inferno Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (838,'Ioplasma, Method for Refining',491,125,1,2400,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (838,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (838,138,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (838,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (838,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (838,79,0,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (838,149,0,5); -- Inferno Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (839,'Cryozine, Method for Refining',492,83,1,2880,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (839,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (839,142,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (839,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (839,79,0,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (839,150,0,5); -- Frigid Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (840,'Cryozine, Method for Refining',492,125,1,2400,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (840,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (840,142,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (840,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (840,79,0,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (840,150,0,5); -- Frigid Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (841,'Magmex, Method for Extracting',493,89,1,2880,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (841,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (841,139,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (841,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (841,114,0,5); -- Shovel
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (841,128,0,5); -- Tongs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (841,147,0,5); -- Lava in Environment
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (842,'Magmex, Method for Extracting',493,125,1,2400,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (842,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (842,139,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (842,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (842,114,0,5); -- Shovel
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (842,128,0,5); -- Tongs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (842,147,0,5); -- Lava in Environment
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (843,'Myrathane, Method for Extracting',494,89,1,2880,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (843,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (843,143,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (843,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (843,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (843,148,0,5); -- Supercooled Liquid in Environment
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (844,'Myrathane, Method for Extracting',494,125,1,2400,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (844,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (844,143,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (844,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (844,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (844,148,0,5); -- Supercooled Liquid in Environment
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (845,'Phlogiston, Method for Drilling',495,51,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (845,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (845,140,1,5); -- Phlogiston
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (845,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (845,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (845,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (846,'Phlogiston, Method for Drilling',495,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (846,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (846,140,1,5); -- Phlogiston
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (846,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (846,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (846,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (847,'Polytaride, Method for Drilling',496,51,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (847,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (847,144,1,5); -- Polytaride
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (847,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (847,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (847,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (848,'Polytaride, Method for Drilling',496,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (848,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (848,144,1,5); -- Polytaride
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (848,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (848,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (848,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (849,'Vulcium, Method for Smelting',497,86,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (849,498,0,5); -- Patent: Smelting Magmium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (849,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (849,145,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (849,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (849,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (849,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (849,137,1,5); -- Vulcanite
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (850,'Vulcium, Method for Smelting',497,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (850,498,0,5); -- Patent: Smelting Magmium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (850,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (850,145,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (850,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (850,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (850,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (850,137,1,5); -- Vulcanite
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (851,'Magmium, Method for Smelting',498,86,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (851,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (851,146,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (851,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (851,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (851,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (851,139,1,5); -- Magmex
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (852,'Magmium, Method for Smelting',498,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (852,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (852,146,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (852,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (852,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (852,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (852,139,1,5); -- Magmex
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (853,'Vulcium Knife, Method for Making using Vulcium and Wood',499,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (853,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (853,152,0,2); -- Vulcium Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (853,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (853,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (853,86,1,5); -- Wood
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (853,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (854,'Vulcium Knife, Method for Making using Vulcium and Wood',499,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (854,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (854,152,0,2); -- Vulcium Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (854,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (854,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (854,86,1,5); -- Wood
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (854,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (855,'Vulcium Knife, Method for Making using Vulcium',499,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (855,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (855,152,0,2); -- Vulcium Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (855,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (855,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (855,145,2,5); -- Vulcium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (856,'Vulcium Knife, Method for Making using Vulcium',499,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (856,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (856,152,0,2); -- Vulcium Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (856,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (856,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (856,145,2,5); -- Vulcium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (857,'Vulcium Knife, Method for Making using Vulcium and Plastic',499,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (857,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (857,152,0,2); -- Vulcium Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (857,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (857,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (857,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (857,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (858,'Vulcium Knife, Method for Making using Vulcium and Plastic',499,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (858,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (858,152,0,2); -- Vulcium Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (858,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (858,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (858,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (858,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (859,'Magmium EV Suit, Method for Making',502,71,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (859,434,0,5); -- Patent: Metal EV Suit
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (859,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (859,153,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (859,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (859,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (859,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (859,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (859,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (859,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (859,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (859,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (859,50,1,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (859,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (859,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (859,146,1,5); -- Magmium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (859,264,1,5); -- Magmium Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (860,'Magmium EV Suit, Method for Making',502,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (860,434,0,5); -- Patent: Metal EV Suit
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (860,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (860,153,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (860,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (860,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (860,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (860,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (860,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (860,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (860,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (860,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (860,50,1,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (860,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (860,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (860,146,1,5); -- Magmium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (860,264,1,5); -- Magmium Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (861,'Vulcium EV Suit, Method for Making',503,71,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (861,502,0,5); -- Patent: Magmium EV Suit
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (861,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (861,155,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (861,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (861,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (861,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (861,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (861,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (861,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (861,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (861,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (861,50,1,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (861,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (861,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (861,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (861,265,1,5); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (862,'Vulcium EV Suit, Method for Making',503,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (862,502,0,5); -- Patent: Magmium EV Suit
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (862,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (862,155,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (862,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (862,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (862,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (862,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (862,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (862,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (862,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (862,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (862,50,1,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (862,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (862,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (862,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (862,265,1,5); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (863,'Vulcium EV Helmet, Method for Making',504,71,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (863,446,0,5); -- Patent: Magmium EV Helmet
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (863,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (863,156,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (863,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (863,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (863,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (863,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (863,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (863,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (863,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (863,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (863,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (863,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (863,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (864,'Vulcium EV Helmet, Method for Making',504,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (864,446,0,5); -- Patent: Magmium EV Helmet
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (864,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (864,156,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (864,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (864,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (864,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (864,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (864,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (864,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (864,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (864,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (864,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (864,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (867,'Oscilloscope, Method for Making',506,75,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (867,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (867,230,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (867,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (867,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (867,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (867,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (867,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (867,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (868,'Oscilloscope, Method for Making',506,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (868,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (868,230,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (868,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (868,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (868,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (868,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (868,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (868,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (869,'Space Fighter F2 Apparatus',507,70,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (869,359,0,4); -- Patent: Space Rocket
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (869,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (869,231,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (869,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (869,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (869,5,2,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (869,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (869,50,8,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (869,51,10,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (869,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (869,102,2,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (869,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (869,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (869,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (869,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (869,226,2,5); -- Grav Coupling for G-drive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (870,'Space Fighter F2 Apparatus',507,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (870,359,0,4); -- Patent: Space Rocket
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (870,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (870,231,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (870,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (870,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (870,5,2,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (870,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (870,50,8,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (870,51,10,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (870,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (870,102,2,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (870,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (870,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (870,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (870,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (870,226,2,5); -- Grav Coupling for G-drive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (871,'Command And Control Module, Method for Making',508,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (871,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (871,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (871,234,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (871,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (871,50,1,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (871,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (871,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (871,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (871,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (871,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (871,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (871,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (872,'Command And Control Module, Method for Making',508,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (872,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (872,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (872,234,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (872,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (872,50,1,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (872,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (872,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (872,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (872,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (872,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (872,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (872,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (873,'Gravity Drive Module, Method for Making',509,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (873,510,0,5); -- Patent: Rocket Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (873,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (873,235,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (873,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (873,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (873,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (873,50,6,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (873,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (873,51,6,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (873,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (873,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (873,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (873,226,2,5); -- Grav Coupling for G-drive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (873,135,0,5); -- Pliers
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (874,'Gravity Drive Module, Method for Making',509,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (874,510,0,5); -- Patent: Rocket Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (874,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (874,235,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (874,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (874,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (874,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (874,50,6,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (874,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (874,51,6,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (874,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (874,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (874,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (874,226,2,5); -- Grav Coupling for G-drive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (874,135,0,5); -- Pliers
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (875,'Rocket Drive Module, Method for Making',510,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (875,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (875,236,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (875,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (875,50,6,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (875,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (875,51,8,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (875,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (875,64,2,5); -- Large rocket motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (875,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (875,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (875,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (876,'Rocket Drive Module, Method for Making',510,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (876,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (876,236,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (876,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (876,50,6,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (876,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (876,51,8,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (876,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (876,64,2,5); -- Large rocket motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (876,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (876,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (876,126,0,5); -- Sewing Needle

INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (879,'Life Support Module, Method for Making',512,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (879,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (879,238,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (879,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (879,5,2,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (879,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (879,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (879,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (879,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (879,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (879,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (879,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (879,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (879,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (879,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (879,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (880,'Life Support Module, Method for Making',512,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (880,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (880,238,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (880,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (880,5,2,5); -- Air
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (880,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (880,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (880,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (880,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (880,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (880,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (880,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (880,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (880,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (880,79,2,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (880,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (881,'Hydrogen Power Plant Module, Method for Making',513,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (881,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (881,239,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (881,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (881,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (881,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (881,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (881,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (881,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (881,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (881,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (881,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (882,'Hydrogen Power Plant Module, Method for Making',513,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (882,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (882,239,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (882,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (882,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (882,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (882,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (882,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (882,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (882,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (882,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (882,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (883,'Energy Shield Module, Method for Making',514,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (883,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (883,240,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (883,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (883,27,5,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (883,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (883,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (883,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (883,227,1,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (883,229,0,5); -- Clamp
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (884,'Energy Shield Module, Method for Making',514,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (884,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (884,240,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (884,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (884,27,5,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (884,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (884,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (884,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (884,227,1,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (884,229,0,5); -- Clamp
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (885,'Kinetic Shield Module, Method for Making',515,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (885,514,0,5); -- Patent: Energy Shield Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (885,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (885,241,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (885,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (885,27,5,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (885,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (885,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (885,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (885,264,1,5); -- Magmium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (885,229,0,5); -- Clamp
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (885,228,1,5); -- Io tube
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (885,230,0,5); -- Oscilloscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (886,'Kinetic Shield Module, Method for Making',515,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (886,514,0,5); -- Patent: Energy Shield Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (886,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (886,241,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (886,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (886,27,5,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (886,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (886,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (886,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (886,264,1,5); -- Magmium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (886,229,0,5); -- Clamp
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (886,228,1,5); -- Io tube
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (886,230,0,5); -- Oscilloscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (887,'Cloaking Shield Module, Method for Making',516,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (887,514,0,5); -- Patent: Energy Shield Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (887,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (887,242,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (887,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (887,27,5,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (887,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (887,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (887,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (887,265,5,5); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (887,229,0,5); -- Clamp
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (887,228,1,5); -- Io tube
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (887,230,0,5); -- Oscilloscope
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (887,269,5,5); -- Tesloid Dissipator
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (887,270,0,5); -- Nanopath Calibrator
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (888,'Cloaking Shield Module, Method for Making',516,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (888,514,0,5); -- Patent: Energy Shield Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (888,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (888,242,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (888,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (888,27,5,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (888,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (888,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (888,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (888,265,5,5); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (888,229,0,5); -- Clamp
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (888,228,1,5); -- Io tube
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (888,230,0,5); -- Oscilloscope
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (888,269,5,5); -- Tesloid Dissipator
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (888,270,0,5); -- Nanopath Calibrator
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (889,'Surgery Medical Module, Method for Making',517,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (889,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (889,243,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (889,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (889,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (889,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (889,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (889,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (889,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (889,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (889,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (889,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (889,79,4,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (889,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (890,'Surgery Medical Module, Method for Making',517,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (890,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (890,243,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (890,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (890,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (890,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (890,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (890,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (890,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (890,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (890,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (890,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (890,79,4,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (890,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (891,'Genetics Medical Module, Method for Making',518,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (891,517,0,5); -- Patent: Surgery Medical Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (891,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (891,244,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (891,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (891,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (891,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (891,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (891,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (891,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (891,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (891,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (891,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (891,266,4,5); -- Myrasplicer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (891,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (892,'Genetics Medical Module, Method for Making',518,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (892,517,0,5); -- Patent: Surgery Medical Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (892,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (892,244,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (892,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (892,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (892,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (892,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (892,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (892,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (892,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (892,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (892,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (892,266,4,5); -- Myrasplicer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (892,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (893,'Cybernetics Medical Module, Method for Making',519,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (893,517,0,5); -- Patent: Surgery Medical Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (893,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (893,245,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (893,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (893,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (893,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (893,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (893,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (893,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (893,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (893,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (893,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (893,268,4,5); -- Gigacell
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (893,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (894,'Cybernetics Medical Module, Method for Making',519,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (894,517,0,5); -- Patent: Surgery Medical Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (894,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (894,245,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (894,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (894,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (894,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (894,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (894,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (894,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (894,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (894,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (894,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (894,268,4,5); -- Gigacell
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (894,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (895,'Personnel Transporter Module, Method for Making',520,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (895,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (895,246,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (895,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (895,27,3,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (895,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (895,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (895,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (895,228,1,5); -- Io tube
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (895,230,0,5); -- Oscilloscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (896,'Personnel Transporter Module, Method for Making',520,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (896,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (896,246,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (896,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (896,27,3,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (896,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (896,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (896,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (896,228,1,5); -- Io tube
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (896,230,0,5); -- Oscilloscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (897,'Vehicle Transporter Module, Method for Making',521,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (897,520,0,5); -- Patent: Personnel Transporter Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (897,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (897,247,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (897,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (897,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (897,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (897,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (897,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (897,228,2,5); -- Io tube
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (897,230,0,5); -- Oscilloscope
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (897,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (897,229,0,5); -- Clamp
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (898,'Vehicle Transporter Module, Method for Making',521,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (898,520,0,5); -- Patent: Personnel Transporter Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (898,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (898,247,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (898,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (898,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (898,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (898,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (898,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (898,228,2,5); -- Io tube
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (898,230,0,5); -- Oscilloscope
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (898,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (898,229,0,5); -- Clamp
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (899,'Energy Weapon Turret Module, Method for Making',522,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (899,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (899,248,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (899,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (899,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (899,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (899,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (899,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (899,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (899,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (899,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (899,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (899,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (899,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (899,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (899,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (899,227,1,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (899,229,0,5); -- Clamp
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (900,'Energy Weapon Turret Module, Method for Making',522,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (900,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (900,248,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (900,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (900,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (900,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (900,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (900,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (900,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (900,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (900,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (900,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (900,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (900,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (900,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (900,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (900,227,1,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (900,229,0,5); -- Clamp
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (901,'Weapon Launcher Turret Module, Method for Making',523,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (901,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (901,250,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (901,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (901,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (901,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (901,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (901,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (901,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (901,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (901,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (901,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (901,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (901,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (901,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (901,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (902,'Weapon Launcher Turret Module, Method for Making',523,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (902,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (902,250,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (902,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (902,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (902,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (902,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (902,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (902,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (902,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (902,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (902,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (902,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (902,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (902,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (902,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (903,'Energy Weapon Bay Module, Method for Making',524,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (903,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (903,252,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (903,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (903,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (903,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (903,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (903,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (903,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (903,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (903,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (903,229,0,5); -- Clamp
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (904,'Energy Weapon Bay Module, Method for Making',524,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (904,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (904,252,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (904,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (904,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (904,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (904,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (904,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (904,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (904,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (904,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (904,229,0,5); -- Clamp
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (905,'Weapon Launcher Bay Module, Method for Making',525,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (905,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (905,254,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (905,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (905,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (905,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (905,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (905,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (905,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (905,4,0,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (906,'Weapon Launcher Bay Module, Method for Making',525,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (906,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (906,254,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (906,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (906,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (906,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (906,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (906,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (906,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (906,4,0,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (907,'Atmosphere Condenser Bay Module, Method for Making',526,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (907,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (907,256,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (907,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (907,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (907,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (907,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (907,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (907,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (907,4,0,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (908,'Atmosphere Condenser Bay Module, Method for Making',526,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (908,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (908,256,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (908,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (908,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (908,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (908,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (908,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (908,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (908,4,0,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (909,'Prospector Bay Module, Method for Making',527,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (909,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (909,257,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (909,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (909,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (909,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (909,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (909,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (909,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (909,4,0,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (910,'Prospector Bay Module, Method for Making',527,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (910,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (910,257,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (910,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (910,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (910,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (910,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (910,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (910,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (910,4,0,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (911,'Tractor Beam Bay Module, Method for Making',528,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (911,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (911,258,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (911,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (911,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (911,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (911,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (911,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (911,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (911,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (911,227,1,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (911,229,0,5); -- Clamp
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (911,228,2,5); -- Io tube
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (911,230,0,5); -- Oscilloscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (912,'Tractor Beam Bay Module, Method for Making',528,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (912,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (912,258,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (912,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (912,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (912,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (912,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (912,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (912,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (912,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (912,227,1,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (912,229,0,5); -- Clamp
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (912,228,2,5); -- Io tube
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (912,230,0,5); -- Oscilloscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (913,'Wormhole P FTL Drive Module, Method for Making',529,110,1,3600*8*1.2,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (913,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (913,259,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (913,157,10000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (913,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (913,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (913,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (913,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (913,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (913,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (913,111,1,5); -- Lumenite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (913,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (914,'Wormhole P FTL Drive Module, Method for Making',529,125,1,3600*8,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (914,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (914,259,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (914,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (914,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (914,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (914,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (914,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (914,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (914,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (914,111,1,5); -- Lumenite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (914,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (915,'Warp 1 FTL Drive Module, Method for Making',530,110,1,1.2*86400*7*8,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (915,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (915,260,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (915,157,10000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (915,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (915,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (915,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (915,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (915,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (915,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (915,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (915,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (916,'Warp 1 FTL Drive Module, Method for Making',530,125,1,86400*7*8,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (916,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (916,260,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (916,157,10000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (916,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (916,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (916,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (916,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (916,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (916,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (916,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (916,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (917,'Ballistic Weapon Bay Module, Method for Making',531,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (917,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (917,261,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (917,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (917,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (917,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (917,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (917,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (917,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (917,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (917,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (917,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (918,'Ballistic Weapon Bay Module, Method for Making',531,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (918,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (918,261,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (918,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (918,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (918,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (918,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (918,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (918,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (918,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (918,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (918,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (919,'Ballistic Weapon Turret Module, Method for Making',532,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (919,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (919,262,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (919,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (919,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (919,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (919,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (919,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (919,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (919,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (919,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (919,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (919,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (919,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (919,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (919,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (920,'Ballistic Weapon Turret Module, Method for Making',532,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (920,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (920,262,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (920,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (920,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (920,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (920,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (920,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (920,79,1,5); -- Textiles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (920,126,0,5); -- Sewing Needle
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (920,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (920,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (920,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (920,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (920,102,1,5); -- Heavy Weapon
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (920,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (921,'Sensor Module, Method for Making',533,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (921,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (921,263,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (921,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (921,27,3,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (921,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (921,50,1,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (921,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (921,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (921,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (922,'Sensor Module, Method for Making',533,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (922,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (922,263,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (922,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (922,27,3,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (922,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (922,50,1,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (922,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (922,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (922,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (923,'Magmium Heat Sink, Method for Making',534,79,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (923,419,0,5); -- Patent: Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (923,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (923,264,0,2); -- Magmium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (923,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (923,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (923,146,1,5); -- Magmium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (923,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (923,142,2,5); -- Cryozine
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (924,'Magmium Heat Sink, Method for Making',534,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (924,419,0,5); -- Patent: Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (924,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (924,264,0,2); -- Magmium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (924,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (924,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (924,146,1,5); -- Magmium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (924,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (924,142,2,5); -- Cryozine
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (925,'Vulcium Heat Sink, Method for Making',535,79,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (925,534,0,5); -- Patent: Magmium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (925,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (925,265,0,2); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (925,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (925,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (925,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (925,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (925,142,2,5); -- Cryozine
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (926,'Vulcium Heat Sink, Method for Making',535,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (926,534,0,5); -- Patent: Magmium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (926,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (926,265,0,2); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (926,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (926,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (926,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (926,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (926,142,2,5); -- Cryozine
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (927,'Myrasplicer, Method for Making',536,81,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (927,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (927,266,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (927,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (927,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (927,24,0,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (927,143,2,5); -- Myrathane
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (927,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (928,'Myrasplicer, Method for Making',536,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (928,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (928,266,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (928,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (928,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (928,24,0,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (928,143,2,5); -- Myrathane
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (928,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (929,'Microscope, Method for Making',537,75,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (929,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (929,267,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (929,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (929,24,1,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (929,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (929,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (929,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (929,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (929,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (930,'Microscope, Method for Making',537,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (930,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (930,267,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (930,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (930,24,1,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (930,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (930,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (930,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (930,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (930,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (931,'Gigacell, Method for Making',538,75,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (931,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (931,268,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (931,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (931,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (931,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (931,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (931,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (931,140,1,5); -- Phlogiston
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (932,'Gigacell, Method for Making',538,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (932,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (932,268,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (932,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (932,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (932,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (932,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (932,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (932,140,1,5); -- Phlogiston
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (933,'Tesloid Dissipator, Method for Making',539,75,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (933,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (933,269,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (933,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (933,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (933,24,1,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (933,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (933,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (933,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (933,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (933,135,0,5); -- Pliers
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (933,141,2,5); -- Bolite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (933,270,0,5); -- Nanopath Calibrator
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (934,'Tesloid Dissipator, Method for Making',539,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (934,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (934,269,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (934,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (934,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (934,24,1,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (934,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (934,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (934,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (934,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (934,135,0,5); -- Pliers
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (934,141,2,5); -- Bolite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (934,270,0,5); -- Nanopath Calibrator
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (935,'Nanopath Calibrator, Method for Making',540,75,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (935,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (935,270,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (935,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (935,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (935,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (935,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (935,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (935,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (935,143,1,5); -- Myrathane
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (936,'Nanopath Calibrator, Method for Making',540,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (936,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (936,270,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (936,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (936,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (936,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (936,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (936,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (936,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (936,143,1,5); -- Myrathane

INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (941,'Particle Collector Bay Module, Method for Making',543,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (941,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (941,283,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (941,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (941,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (941,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (941,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (941,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (941,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (941,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (941,269,1,5); -- Tesloid Dissipator
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (941,270,0,5); -- Nanopath Calibrator
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (942,'Particle Collector Bay Module, Method for Making',543,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (942,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (942,283,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (942,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (942,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (942,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (942,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (942,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (942,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (942,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (942,269,1,5); -- Tesloid Dissipator
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (942,270,0,5); -- Nanopath Calibrator
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (943,'DNA Sample Kit, Method for Making',544,81,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (943,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (943,284,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (943,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (943,79,1,5); -- Textile
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (943,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (944,'DNA Sample Kit, Method for Making',544,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (944,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (944,284,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (944,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (944,79,1,5); -- Textile
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (944,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (945,'Wildcatter Bay Module, Method for Making',547,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (945,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (945,291,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (945,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (945,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (945,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (945,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (945,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (945,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (945,4,0,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (946,'Wildcatter Bay Module, Method for Making',547,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (946,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (946,291,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (946,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (946,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (946,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (946,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (946,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (946,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (946,4,0,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (947,'Siphon Bay Module, Method for Making',548,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (947,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (947,289,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (947,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (947,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (947,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (947,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (947,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (947,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (947,4,0,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (948,'Siphon Bay Module, Method for Making',548,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (948,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (948,289,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (948,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (948,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (948,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (948,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (948,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (948,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (948,4,0,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (949,'Laser Rifle, Method for Making',549,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (949,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (949,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (949,296,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (949,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (949,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (949,37,1,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (949,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (949,63,3,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (949,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (949,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (949,229,0,5); -- Clamp
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (950,'Laser Rifle, Method for Making',549,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (950,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (950,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (950,296,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (950,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (950,27,2,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (950,37,1,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (950,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (950,63,3,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (950,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (950,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (950,229,0,5); -- Clamp
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (951,'Small Rocket BlBu Firecracker, Method for Making',550,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (951,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (951,297,0,2); -- Small Rocket BlBu Firecracker
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (951,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (951,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (951,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (951,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (951,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (951,36,1,5); -- Gasoline
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (952,'Small Rocket BlBu Firecracker, Method for Making',550,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (952,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (952,297,0,2); -- Small Rocket BlBu Firecracker
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (952,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (952,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (952,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (952,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (952,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (952,36,1,5); -- Gasoline
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (953,'Large Rocket BlBu Molotov, Method for Making',551,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (953,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (953,298,0,2); -- Large Rocket BlBu Molotov
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (953,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (953,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (953,64,1,5); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (953,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (953,12,1,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (953,36,1,5); -- Gasoline
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (954,'Large Rocket BlBu Molotov, Method for Making',551,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (954,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (954,298,0,2); -- Large Rocket BlBu Molotov
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (954,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (954,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (954,64,1,5); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (954,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (954,12,1,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (954,36,1,5); -- Gasoline
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (955,'Small Missile BlBu Barnburner, Method for Making',552,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (955,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (955,299,0,2); -- Small Missile BlBu Barnburner
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (955,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (955,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (955,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (955,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (955,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (955,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (955,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (955,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (955,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (955,36,1,5); -- Gasoline
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (956,'Small Missile BlBu Barnburner, Method for Making',552,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (956,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (956,299,0,2); -- Small Missile BlBu Barnburner
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (956,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (956,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (956,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (956,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (956,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (956,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (956,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (956,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (956,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (956,36,1,5); -- Gasoline
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (957,'Large Missile BlBu Fireball, Method for Making',553,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (957,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (957,300,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (957,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (957,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (957,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (957,27,14,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (957,64,1,5); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (957,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (957,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (957,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (957,12,1,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (957,36,1,5); -- Gasoline
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (958,'Large Missile BlBu Fireball, Method for Making',553,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (958,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (958,300,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (958,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (958,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (958,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (958,27,14,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (958,64,1,5); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (958,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (958,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (958,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (958,12,1,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (958,36,1,5); -- Gasoline
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (959,'Small Rocket SlPi Meathook, Method for Making',554,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (959,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (959,301,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (959,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (959,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (959,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (959,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (959,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (959,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (960,'Small Rocket SlPi Meathook, Method for Making',554,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (960,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (960,301,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (960,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (960,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (960,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (960,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (960,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (960,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (961,'Large Rocket BuDi Scudpox, Method for Making',555,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (961,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (961,302,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (961,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (961,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (961,64,1,5); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (961,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (961,36,1,5); -- Gasoline
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (961,120,1,5); -- Antitoxin
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (961,12,1,5); -- Bomb
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (962,'Large Rocket BuDi Scudpox, Method for Making',555,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (962,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (962,302,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (962,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (962,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (962,64,1,5); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (962,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (962,36,1,5); -- Gasoline
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (962,120,1,5); -- Antitoxin
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (962,12,1,5); -- Bomb
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (963,'Small Missile CrFr Snowball, Method for Making',556,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (963,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (963,303,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (963,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (963,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (963,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (963,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (963,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (963,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (963,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (963,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (963,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (963,142,1,5); -- Cryozine
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (964,'Small Missile CrFr Snowball, Method for Making',556,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (964,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (964,303,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (964,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (964,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (964,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (964,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (964,69,1,5); -- Small Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (964,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (964,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (964,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (964,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (964,142,1,5); -- Cryozine
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (965,'Large Missile SlPo Sideviper, Method for Making',557,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (965,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (965,304,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (965,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (965,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (965,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (965,27,14,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (965,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (965,64,1,5); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (965,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (965,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (965,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (965,12,1,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (965,119,1,5); -- Antidote
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (966,'Large Missile SlPo Sideviper, Method for Making',557,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (966,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (966,304,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (966,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (966,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (966,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (966,27,14,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (966,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (966,64,1,5); -- Large Rocket Motor
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (966,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (966,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (966,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (966,12,1,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (966,119,1,5); -- Antidote
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (967,'Laser Pistol, Method for Making',558,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (967,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (967,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (967,305,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (967,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (967,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (967,37,1,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (967,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (967,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (967,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (967,227,1,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (967,229,0,5); -- Clamp
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (968,'Laser Pistol, Method for Making',558,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (968,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (968,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (968,305,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (968,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (968,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (968,37,1,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (968,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (968,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (968,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (968,227,1,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (968,229,0,5); -- Clamp
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (969,'Heavy Arms Ammo PiRa, Method for Making',559,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (969,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (969,306,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (969,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (969,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (969,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (969,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (969,66,1,5); -- Radioactives
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (970,'Heavy Arms Ammo PiRa, Method for Making',559,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (970,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (970,306,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (970,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (970,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (970,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (970,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (970,66,1,5); -- Radioactives
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (971,'Heavy Arms Ammo PiBl, Method for Making',560,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (971,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (971,307,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (971,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (971,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (971,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (971,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (971,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (972,'Heavy Arms Ammo PiBl, Method for Making',560,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (972,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (972,307,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (972,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (972,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (972,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (972,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (972,13,1,5); -- Plastic Explosive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (973,'Heavy Arms Ammo PiAc, Method for Making',561,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (973,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (973,308,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (973,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (973,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (973,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (973,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (973,61,1,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (974,'Heavy Arms Ammo PiAc, Method for Making',561,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (974,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (974,308,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (974,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (974,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (974,40,1,5); -- Gun Powder
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (974,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (974,61,1,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (975,'Stasis Beam Lens, Method for Making',562,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (975,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (975,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (975,309,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (975,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (975,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (975,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (975,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (975,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (975,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (975,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (975,144,2,5); -- Polytaride
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (975,264,2,5); -- Magmium Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (976,'Stasis Beam Lens, Method for Making',562,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (976,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (976,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (976,309,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (976,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (976,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (976,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (976,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (976,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (976,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (976,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (976,144,2,5); -- Polytaride
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (976,264,2,5); -- Magmium Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (977,'Neutron Beam Lens, Method for Making',563,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (977,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (977,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (977,310,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (977,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (977,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (977,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (977,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (977,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (977,66,2,5); -- Radioactives
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (977,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (977,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (977,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (978,'Neutron Beam Lens, Method for Making',563,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (978,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (978,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (978,310,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (978,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (978,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (978,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (978,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (978,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (978,66,2,5); -- Radioactives
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (978,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (978,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (978,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (979,'Electron Beam Lens, Method for Making',564,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (979,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (979,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (979,311,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (979,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (979,24,2,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (979,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (979,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (979,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (979,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (979,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (979,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (979,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (980,'Electron Beam Lens, Method for Making',564,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (980,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (980,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (980,311,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (980,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (980,24,2,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (980,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (980,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (980,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (980,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (980,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (980,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (980,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (981,'Erg Ray Lens, Method for Making',565,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (981,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (981,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (981,312,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (981,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (981,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (981,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (981,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (981,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (981,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (981,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (981,228,2,5); -- Io Tube
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (981,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (982,'Erg Ray Lens, Method for Making',565,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (982,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (982,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (982,312,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (982,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (982,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (982,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (982,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (982,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (982,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (982,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (982,228,2,5); -- Io Tube
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (982,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (983,'Freeze Ray Lens, Method for Making',566,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (983,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (983,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (983,313,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (983,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (983,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (983,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (983,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (983,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (983,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (983,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (983,142,2,5); -- Cryozine
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (983,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (984,'Freeze Ray Lens, Method for Making',566,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (984,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (984,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (984,313,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (984,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (984,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (984,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (984,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (984,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (984,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (984,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (984,142,2,5); -- Cryozine
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (984,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (985,'Compressor Beam Lens, Method for Making',567,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (985,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (985,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (985,314,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (985,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (985,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (985,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (985,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (985,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (985,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (985,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (985,265,2,5); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (985,272,2,5); -- Borexino Precipitate
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (986,'Compressor Beam Lens, Method for Making',567,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (986,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (986,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (986,314,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (986,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (986,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (986,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (986,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (986,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (986,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (986,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (986,265,2,5); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (986,272,2,5); -- Borexino Precipitate
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (987,'Disruptor Beam Lens, Method for Making',568,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (987,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (987,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (987,315,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (987,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (987,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (987,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (987,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (987,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (987,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (987,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (987,143,2,5); -- Myrathane
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (987,264,2,5); -- Magmium Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (988,'Disruptor Beam Lens, Method for Making',568,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (988,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (988,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (988,315,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (988,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (988,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (988,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (988,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (988,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (988,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (988,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (988,143,2,5); -- Myrathane
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (988,264,2,5); -- Magmium Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (989,'Heat Ray Lens, Method for Making',569,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (989,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (989,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (989,316,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (989,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (989,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (989,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (989,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (989,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (989,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (989,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (989,140,2,5); -- Phlogiston
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (989,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (990,'Heat Ray Lens, Method for Making',569,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (990,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (990,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (990,316,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (990,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (990,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (990,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (990,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (990,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (990,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (990,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (990,140,2,5); -- Phlogiston
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (990,227,2,5); -- Cryo Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (991,'Proton Beam Lens, Method for Making',570,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (991,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (991,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (991,317,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (991,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (991,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (991,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (991,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (991,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (991,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (991,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (991,265,2,5); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (991,271,2,5); -- Antiflux Particles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (992,'Proton Beam Lens, Method for Making',570,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (992,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (992,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (992,317,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (992,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (992,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (992,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (992,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (992,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (992,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (992,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (992,265,2,5); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (992,271,2,5); -- Antiflux Particles
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (993,'Disintegrator Ray Lens, Method for Making',571,77,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (993,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (993,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (993,318,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (993,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (993,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (993,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (993,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (993,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (993,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (993,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (993,141,2,5); -- Bolite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (993,265,2,5); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (994,'Disintegrator Ray Lens, Method for Making',571,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (994,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (994,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (994,318,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (994,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (994,27,8,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (994,37,2,5); -- Gems
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (994,51,4,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (994,63,6,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (994,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (994,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (994,141,2,5); -- Bolite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (994,265,2,5); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (995,'Weapon Deployment Bay Module, Method for Making',572,110,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (995,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (995,319,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (995,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (995,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (995,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (995,50,3,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (995,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (995,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (995,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (995,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (996,'Weapon Deployment Bay Module, Method for Making',572,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (996,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (996,319,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (996,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (996,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (996,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (996,50,3,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (996,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (996,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (996,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (996,136,0,5); -- Torque Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (997,'Bomb BlRa Los Alamos, Method for Making',573,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (997,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (997,320,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (997,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (997,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (997,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (997,12,2,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (997,66,100,5); -- Radioactives
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (997,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (998,'Bomb BlRa Los Alamos, Method for Making',573,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (998,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (998,320,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (998,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (998,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (998,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (998,12,2,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (998,66,100,5); -- Radioactives
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (998,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (999,'Bomb BlPi Porcupine, Method for Making',574,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (999,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (999,321,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (999,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (999,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (999,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (999,12,2,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (999,51,100,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1000,'Bomb BlPi Porcupine, Method for Making',574,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1000,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1000,321,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1000,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1000,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1000,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1000,12,2,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1000,51,100,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1001,'Bomb ElSt Tesla, Method for Making',575,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1001,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1001,322,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1001,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1001,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1001,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1001,12,2,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1001,24,25,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1001,51,50,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1001,53,25,5); -- Minerals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1002,'Bomb ElSt Tesla, Method for Making',575,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1002,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1002,322,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1002,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1002,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1002,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1002,12,2,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1002,24,25,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1002,51,50,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1002,53,25,5); -- Minerals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1003,'Bomb SuPo Muffler, Method for Making',576,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1003,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1003,323,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1003,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1003,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1003,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1003,12,2,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1003,61,50,5); -- Petrochemicals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1003,119,50,5); -- Antidote
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1004,'Bomb SuPo Muffler, Method for Making',576,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1004,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1004,323,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1004,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1004,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1004,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1004,12,2,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1004,61,50,5); -- Petrochemicals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1004,119,50,5); -- Antidote
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1005,'Bomb SlCr Meat Slicer, Method for Making',577,80,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1005,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1005,324,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1005,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1005,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1005,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1005,12,2,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1005,51,100,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1006,'Bomb SlCr Meat Slicer, Method for Making',577,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1006,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1006,324,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1006,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1006,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1006,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1006,12,2,5); -- Bomb
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1006,51,100,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1011,'Genesis Device, Method for Making',580,0,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,518,0,5); -- Patent: Genetics Medical Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,328,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,3,1,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,51,6,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,63,8,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,141,1,5); -- Bolite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,142,3,5); -- Cryozine
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,144,2,5); -- Polytaride
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,266,21,5); -- Myrasplicer (210 animal species/zone)
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,267,0,5); -- Microscope
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,268,1,5); -- Gigacell
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,270,0,5); -- Nanopath Calibrator
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1011,272,6,5); -- Borexino Precipitate (54 plant species/zone)
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1012,'Genesis Device, Method for Making',580,0,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,518,0,5); -- Patent: Genetics Medical Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,161,1,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,328,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,3,1,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,51,6,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,63,8,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,141,1,5); -- Bolite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,142,3,5); -- Cryozine
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,144,2,5); -- Polytaride
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,266,21,5); -- Myrasplicer (210 animal species/zone)
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,267,0,5); -- Microscope
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,268,1,5); -- Gigacell
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,270,0,5); -- Nanopath Calibrator
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1012,272,6,5); -- Borexino Precipitate (54 plant species/zone)
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1019,'Candle, Method for Making using Petrochemicals',584,82,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1019,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1019,333,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1019,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1019,61,1,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1020,'Candle, Method for Making using Petrochemicals',584,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1020,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1020,333,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1020,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1020,61,1,5); -- Petrochemicals
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1021,'Preon Ring, Method for Making using Type O Preon',371,78,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1021,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1021,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1021,334,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1021,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1021,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1021,273,2,5); -- Type O Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1022,'Preon Ring, Method for Making using Type O Preon',371,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1022,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1022,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1022,334,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1022,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1022,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1022,273,2,5); -- Type O Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1023,'Preon Ring, Method for Making using Type B Preon',371,78,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1023,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1023,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1023,335,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1023,3,2,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1023,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1023,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1023,274,2,5); -- Type B Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1024,'Preon Ring, Method for Making using Type B Preon',371,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1024,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1024,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1024,335,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1024,3,2,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1024,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1024,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1024,274,2,5); -- Type B Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1025,'Preon Ring, Method for Making using Type A Preon',371,78,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1025,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1025,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1025,336,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1025,3,2,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1025,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1025,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1025,275,2,5); -- Type A Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1026,'Preon Ring, Method for Making using Type A Preon',371,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1026,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1026,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1026,336,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1026,3,2,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1026,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1026,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1026,275,2,5); -- Type A Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1027,'Preon Ring, Method for Making using Type F Preon',371,78,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1027,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1027,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1027,337,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1027,3,2,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1027,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1027,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1027,276,2,5); -- Type F Preon
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1028,'Preon Ring, Method for Making using Type F Preon',371,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1028,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1028,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1028,337,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1028,3,2,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1028,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1028,145,1,5); -- Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1028,276,2,5); -- Type F Preon
-- End of main chunk of Patent RnD processes.
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1029,'Wormhole PN FTL Drive Module',589,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1029,599,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1029,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1029,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1029,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1029,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1029,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1029,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1029,111,1,5); -- Lumenite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1029,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1030,'Wormhole PNN FTL Drive Module',590,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1030,600,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1030,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1030,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1030,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1030,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1030,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1030,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1030,111,1,5); -- Lumenite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1030,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1031,'Warp 2 FTL Drive Module',591,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1031,601,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1031,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1031,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1031,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1031,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1031,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1031,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1031,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1031,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1032,'Warp 3 FTL Drive Module',592,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1032,602,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1032,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1032,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1032,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1032,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1032,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1032,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1032,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1032,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1033,'Warp 4 FTL Drive Module',593,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1033,603,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1033,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1033,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1033,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1033,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1033,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1033,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1033,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1033,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1034,'Warp 5 FTL Drive Module',594,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1034,604,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1034,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1034,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1034,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1034,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1034,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1034,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1034,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1034,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1035,'Warp 6 FTL Drive Module',595,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1035,605,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1035,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1035,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1035,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1035,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1035,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1035,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1035,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1035,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1036,'Warp 7 FTL Drive Module',596,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1036,606,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1036,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1036,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1036,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1036,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1036,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1036,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1036,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1036,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1037,'Warp 8 FTL Drive Module',597,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1037,607,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1037,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1037,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1037,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1037,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1037,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1037,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1037,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1037,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1038,'Warp 9 FTL Drive Module',598,110,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1038,608,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1038,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1038,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1038,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1038,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1038,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1038,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1038,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1038,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1039,'Wormhole PN FTL Drive Module, Method for Making',599,110,1,86400*8*1.2,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1039,529,0,5); -- Patent: Wormhole P FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1039,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1039,589,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1039,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1039,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1039,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1039,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1039,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1039,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1039,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1039,111,1,5); -- Lumenite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1039,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1040,'Wormhole PN FTL Drive Module, Method for Making',599,125,1,86400*8,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1040,529,0,5); -- Patent: Wormhole P FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1040,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1040,589,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1040,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1040,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1040,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1040,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1040,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1040,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1040,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1040,111,1,5); -- Lumenite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1040,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1041,'Wormhole PNN FTL Drive Module, Method for Making',600,110,1,86400*8*2*1.2,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1041,599,0,5); -- Patent: Wormhole PN FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1041,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1041,590,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1041,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1041,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1041,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1041,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1041,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1041,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1041,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1041,111,1,5); -- Lumenite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1041,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1042,'Wormhole PNN FTL Drive Module, Method for Making',600,125,1,86400*8*2,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1042,599,0,5); -- Patent: Wormhole PN FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1042,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1042,590,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1042,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1042,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1042,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1042,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1042,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1042,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1042,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1042,111,1,5); -- Lumenite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1042,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1043,'Warp 2 FTL Drive Module, Method for Making',601,110,1,1.2*86400*7*8*2,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1043,530,0,5); -- Patent: Warp 1 FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1043,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1043,591,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1043,157,10000*2,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1043,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1043,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1043,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1043,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1043,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1043,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1043,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1043,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1044,'Warp 2 FTL Drive Module, Method for Making',601,125,1,86400*7*8*2,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1044,530,0,5); -- Patent: Warp 1 FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1044,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1044,591,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1044,157,10000*2,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1044,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1044,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1044,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1044,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1044,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1044,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1044,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1044,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1045,'Warp 3 FTL Drive Module, Method for Making',602,110,1,1.2*86400*7*8*3,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1045,601,0,5); -- Patent: Warp 2 FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1045,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1045,592,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1045,157,10000*3,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1045,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1045,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1045,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1045,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1045,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1045,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1045,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1045,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1046,'Warp 3 FTL Drive Module, Method for Making',602,125,1,86400*7*8*3,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1046,601,0,5); -- Patent: Warp 2 FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1046,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1046,592,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1046,157,10000*3,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1046,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1046,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1046,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1046,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1046,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1046,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1046,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1046,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1047,'Warp 4 FTL Drive Module, Method for Making',603,110,1,1.2*86400*7*8*4,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1047,602,0,5); -- Patent: Warp 3 FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1047,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1047,593,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1047,157,10000*4,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1047,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1047,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1047,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1047,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1047,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1047,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1047,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1047,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1048,'Warp 4 FTL Drive Module, Method for Making',603,125,1,86400*7*8*4,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1048,602,0,5); -- Patent: Warp 3 FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1048,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1048,593,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1048,157,10000*4,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1048,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1048,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1048,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1048,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1048,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1048,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1048,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1048,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1049,'Warp 5 FTL Drive Module, Method for Making',604,110,1,1.2*86400*7*8*5,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1049,603,0,5); -- Patent: Warp 4 FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1049,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1049,594,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1049,157,10000*5,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1049,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1049,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1049,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1049,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1049,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1049,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1049,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1049,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1050,'Warp 5 FTL Drive Module, Method for Making',604,125,1,86400*7*8*5,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1050,603,0,5); -- Patent: Warp 4 FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1050,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1050,594,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1050,157,10000*5,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1050,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1050,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1050,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1050,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1050,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1050,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1050,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1050,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1051,'Warp 6 FTL Drive Module, Method for Making',605,110,1,1.2*86400*7*8*6,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1051,604,0,5); -- Patent: Warp 5 FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1051,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1051,595,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1051,157,10000*6,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1051,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1051,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1051,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1051,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1051,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1051,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1051,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1051,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1052,'Warp 6 FTL Drive Module, Method for Making',605,125,1,86400*7*8*6,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1052,604,0,5); -- Patent: Warp 5 FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1052,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1052,595,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1052,157,10000*6,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1052,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1052,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1052,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1052,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1052,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1052,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1052,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1052,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1053,'Warp 7 FTL Drive Module, Method for Making',606,110,1,1.2*86400*7*8*7,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1053,605,0,5); -- Patent: Warp 6 FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1053,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1053,596,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1053,157,10000*7,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1053,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1053,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1053,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1053,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1053,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1053,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1053,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1053,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1054,'Warp 7 FTL Drive Module, Method for Making',606,125,1,86400*7*8*7,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1054,605,0,5); -- Patent: Warp 6 FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1054,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1054,596,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1054,157,10000*7,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1054,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1054,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1054,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1054,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1054,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1054,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1054,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1054,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1055,'Warp 8 FTL Drive Module, Method for Making',607,110,1,1.2*86400*7*8*8,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1055,606,0,5); -- Patent: Warp 7 FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1055,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1055,597,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1055,157,10000*8,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1055,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1055,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1055,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1055,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1055,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1055,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1055,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1055,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1056,'Warp 8 FTL Drive Module, Method for Making',607,125,1,86400*7*8*8,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1056,606,0,5); -- Patent: Warp 7 FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1056,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1056,597,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1056,157,10000*8,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1056,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1056,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1056,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1056,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1056,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1056,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1056,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1056,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1057,'Warp 9 FTL Drive Module, Method for Making',608,110,1,1.2*86400*7*8*9,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1057,607,0,5); -- Patent: Warp 8 FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1057,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1057,598,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1057,157,10000*9,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1057,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1057,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1057,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1057,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1057,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1057,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1057,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1057,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1058,'Warp 9 FTL Drive Module, Method for Making',608,125,1,86400*7*8*9,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1058,607,0,5); -- Patent: Warp 8 FTL Drive Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1058,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1058,598,0,2); -- 
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1058,157,10000*9,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1058,27,6,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1058,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1058,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1058,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1058,63,2,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1058,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1058,271,1,5); -- Antiflux Particles
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1058,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1059,'Green Binder using Paper',421,140,1,10,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1059,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1059,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1059,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1059,282,2,5); -- Paper
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1060,'Green Binder using Plastic',421,82,1,10,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1060,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1060,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1060,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1060,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1061,'Electricity, Method for Generating using Coal',391,60,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1061,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1061,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1061,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1061,22,1,5); -- Coal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1061,114,0,5); -- Shovel
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1062,'Electricity, Method for Generating using Gasoline',392,61,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1062,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1062,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1062,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1062,36,1,5); -- Gasoline
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1062,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1063,'Electricity, Method for Generating using Hydrogen',393,63,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1063,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1063,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1063,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1063,44,1,5); -- Hydrogen
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1063,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1064,'Electricity, Method for Generating using Natural Gas',394,64,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1064,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1064,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1064,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1064,57,1,5); -- Natural Gas
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1064,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1065,'Electricity, Method for Generating using Sunlight',395,66,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1065,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1065,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1065,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1065,94,0,5); -- Sunlight
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1066,'Electricity, Method for Generating using Wind',396,67,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1066,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1066,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1066,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1066,87,0,5); -- Atmosphere Density
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1067,'Electricity, Method for Generating using Radioactives',397,0,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1067,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1067,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1067,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1067,66,1,5); -- Radioactives
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1067,128,0,5); -- Tongs
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1068,'Electricity, Method for Generating using Water in Environment',413,62,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1068,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1068,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1068,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1068,88,0,5); -- Water in Environment needed but not consumed
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1069,'Adamantite',609,50,2,785,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1069,613,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1069,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1069,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1069,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1069,25,1,6); -- Dynamite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1069,82,0,6); -- SUV
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1070,'Adamantite, Method for Mining',613,50,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1070,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1070,609,1,5); -- Adamantite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1070,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1070,4,0,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1071,'Adamantite, Method for Mining',613,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1071,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1071,609,1,5); -- Adamantite
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1071,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1071,4,0,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1072,'Adamantine',610,86,1,751);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1072,614,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1072,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1072,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1072,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1072,265,0,5); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1072,269,0,5); -- Tesloid Dissipator
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1072,609,1,5); -- Adamantite
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1073,'Adamantine, Method for Smelting',614,86,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1073,497,0,5); -- Patent: Smelting Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1073,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1073,610,0,2); -- Adamantine
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1073,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1073,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1073,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1073,265,0,5); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1073,269,0,5); -- Tesloid Dissipator
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1073,609,1,5); -- Adamantite
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1074,'Adamantine, Method for Smelting',614,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1074,497,0,5); -- Patent: Smelting Vulcium
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1074,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1074,610,0,2); -- Adamantine
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1074,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1074,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1074,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1074,265,0,5); -- Vulcium Heat Sink
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1074,269,0,5); -- Tesloid Dissipator
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1074,609,1,5); -- Adamantite
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1075,'Viathol',611,51,1,350,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1075,615,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1075,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1075,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1075,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1076,'Viathol, Method for Drilling',615,51,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1076,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1076,611,1,5); -- Viathol
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1076,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1076,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1076,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1077,'Viathol, Method for Drilling',615,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1077,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1077,611,1,5); -- Viathol
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1077,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1077,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1077,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1078,'Flomentum',612,89,1,375,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1078,616,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1078,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1078,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1078,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1078,148,0,5); -- Supercooled Liquid in Environment
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (1079,'Flomentum, Method for Extracting',616,89,1,2880,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1079,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1079,612,0,2); -- Flomentum
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1079,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1079,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1079,148,0,5); -- Supercooled Liquid in Environment
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,nat) VALUES (1080,'Flomentum, Method for Extracting',616,125,1,2400,0,'f');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1080,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1080,612,0,2); -- Flomentum
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1080,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1080,123,0,5); -- Fitting Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1080,148,0,5); -- Supercooled Liquid in Environment
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1081,'Storage Crystal',617,75,4,12);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1081,618,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1081,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1081,3,1,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1081,24,1,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1081,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1081,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1081,143,1,5); -- Myrathane
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1081,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1081,267,0,5); -- Microscope
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1081,270,0,5); -- Nanopath Calibrator
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1082,'Storage Crystal, Method for Making',618,75,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1082,620,0,5); -- Patent: Thumb Drive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1082,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1082,3,1,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1082,24,1,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1082,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1082,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1082,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1082,143,1,5); -- Myrathane
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1082,267,0,5); -- Microscope
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1082,270,0,5); -- Nanopath Calibrator
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1082,617,0,2); -- Storage Crystal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1083,'Storage Crystal, Method for Making',618,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1083,620,0,5); -- Patent: Thumb Drive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1083,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1083,3,1,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1083,24,1,5); -- Crystals
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1083,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1083,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1083,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1083,143,1,5); -- Myrathane
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1083,267,0,5); -- Microscope
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1083,270,0,5); -- Nanopath Calibrator
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1083,617,0,2); -- Storage Crystal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1084,'Thumb Drive',619,75,4,12);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1084,620,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1084,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1084,3,1,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1084,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1084,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1084,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1084,267,0,5); -- Microscope
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1085,'Thumb Drive, Method for Making',620,75,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1085,463,0,5); -- Patent: Blank Disk
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1085,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1085,3,1,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1085,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1085,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1085,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1085,267,0,5); -- Microscope
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1085,619,0,2); -- Thumb Drive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1086,'Thumb Drive, Method for Making',620,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1086,463,0,5); -- Patent: Blank Disk
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1086,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1086,3,1,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1086,27,1,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1086,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1086,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1086,267,0,5); -- Microscope
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1086,619,0,2); -- Thumb Drive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1087,'Jewelry Storage Device, Method for Making using Thumb Drive',621,78,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1087,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1087,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1087,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1087,98,1,5); -- Gold Bracelet
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1087,619,1,5); -- Thumb Drive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1088,'Jewelry Storage Device, Method for Making using Thumb Drive',621,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1088,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1088,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1088,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1088,98,1,5); -- Gold Bracelet
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1088,619,1,5); -- Thumb Drive
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1089,'Jewelry Storage Device, Method for Making using Storage Crystal',621,78,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1089,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1089,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1089,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1089,98,1,5); -- Gold Bracelet
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1089,617,1,5); -- Storage Crystal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1090,'Jewelry Storage Device, Method for Making using Storage Crystal',621,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1090,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1090,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1090,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1090,98,1,5); -- Gold Bracelet
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1090,617,1,5); -- Storage Crystal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1091,'Thumb Drive Bracelet',622,78,2,12);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1091,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1091,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1091,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1091,98,1,5); -- Gold Bracelet
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1091,619,1,5); -- Thumb Drive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1091,621,0,5); -- Jewelry Storage Device Patent
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1092,'Storage Crystal Bracelet',623,78,2,36);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1092,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1092,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1092,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1092,98,1,5); -- Gold Bracelet
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1092,617,1,5); -- Storage Crystal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1092,621,0,5); -- Jewelry Storage Device Patent
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1093,'Thumb Drive Necklace',624,78,2,16);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1093,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1093,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1093,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1093,46,1,5); -- Crystal Necklace
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1093,619,1,5); -- Thumb Drive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1093,621,0,5); -- Jewelry Storage Device Patent
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1094,'Storage Crystal Necklace',625,78,2,48);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1094,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1094,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1094,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1094,46,1,5); -- Crystal Necklace
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1094,617,1,5); -- Storage Crystal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1094,621,0,5); -- Jewelry Storage Device Patent
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1095,'Thumb Drive Ring',626,78,2,20);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1095,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1095,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1095,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1095,99,1,5); -- Gem Ring
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1095,619,1,5); -- Thumb Drive
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1095,621,0,5); -- Jewelry Storage Device Patent
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1096,'Storage Crystal Ring',627,78,2,60);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1096,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1096,3,0,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1096,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1096,99,1,5); -- Gem Ring
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1096,617,1,5); -- Storage Crystal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1096,621,0,5); -- Jewelry Storage Device Patent
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1097,'Coal Power Plant Module',628,0,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1097,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1097,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1097,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1097,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1097,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1097,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1097,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1097,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1098,'Gasoline Power Plant Module',629,0,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1098,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1098,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1098,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1098,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1098,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1098,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1098,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1098,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1099,'Hydroelectric Power Plant Module',630,0,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1099,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1099,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1099,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1099,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1099,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1099,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1099,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1099,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1100,'Natural Gas Power Plant Module',631,0,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1100,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1100,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1100,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1100,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1100,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1100,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1100,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1100,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1101,'Nuclear Power Plant Module',632,0,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1101,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1101,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1101,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1101,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1101,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1101,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1101,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1101,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1102,'Solar Power Plant Module',633,0,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1102,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1102,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1102,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1102,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1102,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1102,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1102,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1102,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1103,'Wind Power Plant Module',634,0,1,100);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1103,27,4,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1103,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1103,51,2,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1103,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1103,50,2,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1103,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1103,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1103,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1104,'Wood Hull',635,0,1,100); -- Upgrade Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1104,86,1,5); -- Log
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1104,4,1,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1105,'Lumber Hull',636,0,1,100); -- Upgrade Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1105,86,1,5); -- Log
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1105,4,1,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1106,'Stone Hull',637,0,1,100); -- Upgrade Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1106,86,1,5); -- Log
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1106,4,1,5); -- Hammer
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1107,'Adamantine Hull',638,0,1,1000); -- Upgrade Module
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1107,27,5,5); -- Electronic Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1107,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1107,50,5,5); -- Mechanical Part
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1107,129,0,5); -- Wrench
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1107,610,5,5); -- Adamantine
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1107,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1107,63,5,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1107,124,0,5); -- Glue Gun

INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1110,'Blue Binder using Paper',422,140,1,10,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1110,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1110,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1110,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1110,282,2,5); -- Paper
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1111,'Blue Binder using Plastic',422,82,1,10,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1111,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1111,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1111,124,0,2); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1111,63,1,5); -- Plastic

INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1120,'Building Blueprint Disk using Metal',651,0,20,4,0); -- Building Blueprint Disk
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1120,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1120,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1120,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1120,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1121,'Building Blueprint Disk using Plastic',651,0,20,4,0); -- Building Blueprint Disk
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1121,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1121,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1121,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1121,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1122,'Building Blueprint Drawing using Lumber',650,0,10,10,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1122,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1122,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1122,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1122,48,1,5); -- Lumber
SELECT * FROM b_ManufacturingProcess;
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor) VALUES (1123,'Nuclear Fuel Rod',652,90,1,200);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1123,653,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1123,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1123,3,1,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1123,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1123,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1123,66,10,5); -- Radioactives
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1123,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1123,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1123,229,0,5); -- Clamp
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1124,'Nuclear Fuel Rod, Method for Making',653,90,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1124,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1124,3,1,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1124,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1124,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1124,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1124,66,10,5); -- Radioactives
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1124,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1124,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1124,229,0,5); -- Clamp
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1124,652,0,2); -- Nuclear Fuel Rod
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1125,'Nuclear Fuel Rod, Method for Making',653,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1125,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1125,3,1,5); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1125,51,1,5); -- Metal
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1125,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1125,63,1,5); -- Plastic
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1125,66,10,5); -- Radioactives
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1125,125,0,5); -- Screw Driver
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1125,127,0,5); -- Solder Iron
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1125,229,0,5); -- Clamp
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1125,652,0,2); -- Nuclear Fuel Rod

INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (1127,'Plutonium',655,68,1,1000,0,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1127,657,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1127,161,0,5); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1127,128,0,5); -- Tongs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1127,652,2,5); -- Nuclear Fuel Rod
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft,isdefault) VALUES (1128,'Electricity using Nuclear Fuel Rod',3,68,100,4,0,'t');
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1128,656,0,4); -- Patent
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1128,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1128,128,0,5); -- Tongs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1128,652,1,5); -- Nuclear Fuel Rod
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1129,'Electricity, Method for Generating using Nuclear Fuel Rod',656,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1129,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1129,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1129,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1129,128,0,5); -- Tongs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1129,652,1,5); -- Nuclear Fuel Rod
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1130,'Electricity, Method for Generating using Nuclear Fuel Rod',656,68,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1130,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1130,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1130,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1130,128,0,5); -- Tongs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1130,652,1,5); -- Nuclear Fuel Rod
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1131,'Plutonium, Method for Making',657,125,1,2400,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1131,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1131,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1131,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1131,128,0,5); -- Tongs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1131,652,2,5); -- Nuclear Fuel Rod
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1132,'Plutonium, Method for Making',657,68,1,2880,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1132,161,0,2); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1132,3,0,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1132,157,1000,5); -- Money
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1132,128,0,5); -- Tongs
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1132,652,2,5); -- Nuclear Fuel Rod

-- The next three processes are for spacecraft titles. The industry is 0 so they can't be manufactured directly.
-- They are here so the cost analyzer can determine their base price.
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1138,'Spacecraft Title using Metal',663,0,20,4,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1138,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1138,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1138,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1138,51,1,5); -- Metal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1139,'Spacecraft Title using Plastic',663,0,20,4,0);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1139,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1139,3,1,6); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1139,124,0,5); -- Glue Gun
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1139,63,1,5); -- Plastic
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1140,'Spacecraft Title using Lumber',663,0,10,10,0); -- Piece of paper aka Spacecraft Title
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1140,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1140,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1140,4,0,5); -- Hammer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1140,48,1,5); -- Lumber
-- End of data disk bogus processes.
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1145,'Brain of Animal',373,45,1,5,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1145,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1145,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1145,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1145,14,1,5); -- Animal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1146,'Brain of Fish',373,45,1,5,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1146,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1146,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1146,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1146,33,1,5); -- Fish
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1147,'Heart of Animal',385,45,1,5,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1147,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1147,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1147,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1147,14,1,5); -- Animal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1148,'Heart of Fish',385,45,1,5,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1148,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1148,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1148,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1148,33,1,5); -- Fish
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1149,'Stomach of Animal',405,45,1,5,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1149,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1149,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1149,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1149,14,1,5); -- Animal
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1150,'Stomach of Fish',405,45,1,5,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1150,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1150,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1150,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1150,33,1,5); -- Fish
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1151,'Gills of Fish',382,45,1,5,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1151,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1151,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1151,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1151,33,1,5); -- Fish
INSERT INTO b_ManufacturingProcess (id,name,idc,idi,qty,labor,craft) VALUES (1152,'Lungs of Animal',404,45,1,5,1);
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1152,161,0,6); -- Computer
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1152,3,1,2); -- Electricity
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1152,116,0,2); -- Knife
INSERT INTO b_ManufacturingComponent (idmp,idc,consumed,ide) VALUES (1152,14,1,5); -- Animal
-- End of b_ManufacturingProcess


----- End of Static Data -----
