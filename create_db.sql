-- This needs to go into the dummy.sqlite3 file
-- "<create_db.sql sqlite3 dummy.sqlite3" should do it if you dont already have a dummy.sqlite3 file
PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;
CREATE TABLE monkey ( id int , counter int, mesg text);
INSERT INTO "monkey" VALUES(1,0,'Hello');
INSERT INTO "monkey" VALUES(2,0,'Donkey');
INSERT INTO "monkey" VALUES(3,6,'Monkey');
COMMIT;
