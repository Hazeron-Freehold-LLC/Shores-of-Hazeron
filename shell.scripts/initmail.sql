-------------------
-- Mail Database --
-------------------

-- Database name: auMail
-- Owner: aumaster
-- Password: Light
-- User: audog
-- Password: Gold

-- Table Naming Convention
--
-- g_ Global/Game. Corresponding Dg classes exist.


---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
----- Global Game Data -----

-- In-game E-mail
-- DROP TABLE g_Mail;
CREATE TABLE g_Mail
(
 id                                     bigserial primary key unique,
 tmSent                                 timestamp,              -- Time message was sent.
 idPlayer                               integer,                -- player account id who really sent message
 nameFrom                               text,                   -- name of sender
 idFrom                                 bigint,                 -- atom id of sender
 idTo                                   bigint,                 -- atom id of recipient
 msgType                                smallint,               -- AuMailMsg::MSGTYPE message type. 0=regular mail
 subject                                text,                   -- subject line
 body                                   text,                   -- body text
 fromSystem                             text,                   -- name of sender's solar system, can be blank
 fromSystemId                           bigint,                 -- atom id of sender's solar system, can be 0
 fromWorld                              text,                   -- name of sender's world, can be blank
 fromWorldId                            bigint,                 -- atom id of sender's world, can be 0
 fromGalaxy                             smallint,               -- GALAXY enum value. 15=Hazeron, 17=VeilOfTargoss
 fromGalaxyX                            float4,                 -- Galaxy X coordinate origin of message, in parsecs (pc).
 fromGalaxyY                            float4,                 -- Galaxy Y coordinate origin of message, in parsecs (pc).
 fromGalaxyZ                            float4,                 -- Galaxy Z coordinate origin of message, in parsecs (pc).
 deleted                                boolean                 -- true if message was deleted by client
);
GRANT ALL ON g_Mail TO audog;
GRANT ALL ON g_Mail_id_seq TO audog;
CREATE INDEX g_Mail_id_index ON g_Mail USING btree (id);
CREATE INDEX g_Mail_idTo_index ON g_Mail USING btree (idTo);
CREATE INDEX g_Mail_deleted_index ON g_Mail USING btree (deleted);
CREATE INDEX g_Mail_multi_index ON g_Mail USING btree (deleted, tmSent, msgType);
SELECT * FROM g_Mail;

---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+
