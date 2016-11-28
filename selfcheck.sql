LOAD xsmall FROM 'xsmall.del' WITH INDEX
SELECT * FROM xsmall WHERE key > 500 AND key = 272
SELECT * FROM xsmall WHERE key < 500 AND key = 272
SELECT * FROM xsmall WHERE key = 272 AND key = 3084
SELECT * FROM xsmall WHERE key > 4000
SELECT * FROM xsmall WHERE key <> 272
SELECT * FROM xsmall WHERE key = 272 AND key <> 272
SELECT * FROM xsmall WHERE key = 272 AND key < 272
SELECT * FROM xsmall WHERE key = 272 AND key = 272
SELECT * FROM xsmall WHERE key < 2000 AND key < 2500 AND key < 4000
SELECT * FROM xsmall WHERE key = 272 AND key < 272
SELECT * FROM xsmall WHERE key > 272 AND key <= 1578 AND key < 2500
SELECT * FROM xsmall WHERE key = 272 AND value <> '272'
SELECT * FROM xsmall WHERE key = 272 AND value <> 'Baby Take a Bow'
SELECT * FROM xsmall WHERE key = 272 AND value <> 'Baby Take a Bow' AND value = 'Baby Take a Bow'
