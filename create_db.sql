-- This needs to go into the dummy.sqlite3 file
-- "<create_db.sql sqlite3 dummy.sqlite3" should do it if you dont already have a dummy.sqlite3 file
PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE monkey ( id text , counter int, mesg text);
INSERT INTO "monkey" VALUES("object_a",0,'Hello');
INSERT INTO "monkey" VALUES("object_b",0,'Donkey');
INSERT INTO "monkey" VALUES("object_c",6,'Monkey');
COMMIT;
