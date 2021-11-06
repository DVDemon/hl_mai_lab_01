create user if not exists "stud" identified by "stud";
grant all on stud.* to "stud";
CREATE TABLE IF NOT EXISTS `Person`
(
    `login`  VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
    `first_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
    `last_name` VARCHAR(256) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
    `age` INTEGER NOT NULL,PRIMARY KEY (`login`),
    INDEX `login` (`login`)
);