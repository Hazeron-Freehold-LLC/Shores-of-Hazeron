----------------------------
-- Login Account Database --
----------------------------

-- Database name: auLogin
-- Owner: aumaster
-- Password: Light
-- User: audog
-- Password: Gold

-- Human game player information.
-- Not to be confused with the "a_Person", which is an a_Unit atom controlled
-- by a Player.
-- Class: AuPlayerRec
-- DROP TABLE g_Player;
CREATE TABLE g_Player
(
  id           serial primary key unique,
  name         text,                       -- Name of human account holder.
  passcrypt    bytea,                      -- SHA-1 hash key of account holder's password
  email        text,                       -- E-mail address of human account holder.
  timeCreate   timestamp,                  -- Time this account was created.
  isLoggedIn   boolean default false,      -- true while logged-in.
  confirm      integer default 0,          -- Login confirmation number
  status       integer default 0,          -- Status/type of this account. 0=emperor, 1=closed, 2=server, 3=agent, 4=programmer, 5=architect, 6=minion, 7=trial
  activated    timestamp,                  -- Time this account was activated. i.e. status set to 0 from 1.
  days         smallint default 0,         -- Days from activation that this account will stop being active. i.e. status will be set to 1 from 0.
  neverExpire  boolean default false,      -- true for accounts that never expire
  idu1         bigint default 0,           -- a_Person Unit controlled by this Player.
  idu2         bigint default 0,           -- a_Person Unit controlled by this Player.
  idu3         bigint default 0,           -- a_Person Unit controlled by this Player.
  idu4         bigint default 0,           -- a_Person Unit controlled by this Player.
  idu5         bigint default 0,           -- a_Person Unit controlled by this Player.
  idu6         bigint default 0            -- a_Person Unit controlled by this Player.
);
GRANT ALL ON g_Player TO audog;
GRANT ALL ON g_Player_id_seq TO audog;
CREATE INDEX g_Player_id_index ON g_Player USING btree (id);
CREATE INDEX g_Player_confirm_index ON g_Player USING btree (confirm);
CREATE INDEX g_Player_name_index ON g_Player USING btree (name);
CREATE INDEX g_Player_idu1_index ON g_Player USING btree (idu1);
CREATE INDEX g_Player_idu2_index ON g_Player USING btree (idu2);
CREATE INDEX g_Player_idu3_index ON g_Player USING btree (idu3);
CREATE INDEX g_Player_idu4_index ON g_Player USING btree (idu4);
CREATE INDEX g_Player_idu5_index ON g_Player USING btree (idu5);
CREATE INDEX g_Player_idu6_index ON g_Player USING btree (idu6);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('WebMaster.Hazeron.com', '\x10df53478309bf6f68d1a31e27e86db22936a22c', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u1s0', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u1s1', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u1s2', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u1s3', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u1s4', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u2s0', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u2s1', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u2s2', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u2s3', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u2s4', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u3s0', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u3s1', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u3s2', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u3s3', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u3s4', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u4s0', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u4s1', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u4s2', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u4s3', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u4s4', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u5s0', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u5s1', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u5s2', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u5s3', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u5s4', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u6s0', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u6s1', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u6s2', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u6s3', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u6s4', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u7s0', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u7s1', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u7s2', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u7s3', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u7s4', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u8s0', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u8s1', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u8s2', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u8s3', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u8s4', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u9s0', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u9s1', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u9s2', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u9s3', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u9s4', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u10s0', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u10s1', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u10s2', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u10s3', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u10s4', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u11s0', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u11s1', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u11s2', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u11s3', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u11s4', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u12s0', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u12s1', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u12s2', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u12s3', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u12s4', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u13s0', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u13s1', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u13s2', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u13s3', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u13s4', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u13s5', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u13s6', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u13s7', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u13s8', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u14s0', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u14s1', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u14s2', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u14s3', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u14s4', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u14s5', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u14s6', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u14s7', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u14s8', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u15s0', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u15s1', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u15s2', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u15s3', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u15s4', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u15s5', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u15s6', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u15s7', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u15s8', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u16s0', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u16s1', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u16s2', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u16s3', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u16s4', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u16s5', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u16s6', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u16s7', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('u16s8', '\xd3fe9184ff474b00db34e1e6f0bd227323523360', 2);
INSERT INTO g_Player (name, passcrypt, status) VALUES ('David Hoeft', '\xd5e86f780ae5627d144d6bdec23cf65285c748c6', 5);
SELECT * FROM g_Player;


-- Human player contact list, e.g. buddies and blocked people.
-- Class: DgContact
-- Contacts are related to g_Player records by id (i.e. idp).
-- DROP TABLE g_Contact;
CREATE TABLE g_Contact
(
  idp          integer,                     -- g_Player id of owner
  idu          bigint,                      -- a_Person unit of contact
  name         text,                        -- a_Person unit's name
  dispo        smallint                     -- disposition to contact
                                            -- 0=ok, 1=blocked
);
GRANT ALL ON g_Contact TO audog;
CREATE INDEX g_Contact_idp_index ON g_Contact USING btree (idp);
CREATE INDEX g_Contact_multi_index ON g_Contact USING btree (idp, idu);
SELECT * FROM g_Contact;
