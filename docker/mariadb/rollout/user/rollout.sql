create table _roles (
    id INT NOT NULL AUTO_INCREMENT, 
    name varchar(32) not null unique,
    caption varchar(64) not null,
    PRIMARY KEY(id)
);
insert into _roles (name, caption) values ("admin", "Admin role");

create table _user (
    id INT NOT NULL, 
    login varchar(128) not null unique, 
    password varchar(256) not null, 
    name varchar(256) not null, 
    email varchar(128) not null unique, 
    deleted boolean not null default false,
    PRIMARY KEY(id)
);
create table _users_roles (
    role_name varchar(32) references _roles(name),
    user_id int references _user(id),
    unique(role_name, user_id)
);

-- insert into _user(login, password,  name, email) values ('autotest1', '123', 'Test User 1', 'email@dot1.com');
-- insert into _user(login, password,  name, email) values ('autotest2', '123', 'Test User 2', 'email@dot2.com');
-- insert into _users_roles select r.id, u.id from _user u, _roles r where u.login = 'autotest1' and r.name = 'admin';

-- create table _product (
--     id INT NOT NULL AUTO_INCREMENT,
--     name varchar(256) not null,
--     description varchar(512) not null,
--     cost float not null,
--     seller_id int not null references _user(id),
--     creation_date DATETIME not null default now(),
--     deleted boolean not null default false,
--     PRIMARY KEY(id)
-- );
-- insert into _product (name, description, cost, seller_id) 
--     select
--         "test product 1",
--         "test product 1",
--         100,
--         u.id
--     from _user as u where login = 'autotest1';
-- insert into _product (name, description, cost, seller_id) 
--     select
--         "test product 2",
--         "test product 2",
--         500,
--         u.id
--     from _user as u where login = 'autotest2';