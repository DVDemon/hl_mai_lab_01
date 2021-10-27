sudo mysql -e '
select user from mysql.user;
create user if not exists "stud" identified by "stud";
create schema if not exists stud;
grant all on stud.* to "stud";
drop table if exists stud.users;
create table if not exists stud.users (
  login char(50) not null primary key,
  first_name char(50) not null,
  last_name char(50) not null,
  age int not null
);
describe stud.users;

insert into stud.users values(
  "uta123", "Uriy", "Skakovskiy", 23
);
select * from stud.users;
'
