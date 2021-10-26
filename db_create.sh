sudo mysql -e '
select user from mysql.user;
create user if not exists "stud" identified by "stud";
create schema if not exists stud;
grant all on stud.* to "stud";
create table if not exists stud.users (
  login char(50) not null primary key
);
describe stud.users;
'
