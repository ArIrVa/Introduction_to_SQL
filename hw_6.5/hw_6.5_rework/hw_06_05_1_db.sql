create table client (
	id_client SERIAL primary key,
	name text,
	surname text,
	email text
);

--create table client (
--	name text,
--	surname text,
--	email text,
--	primary key (name, surname)
--);

create table phone (
	phone text,
	id_client integer references client (id_client),
	primary key (id_client, phone)
);

--create table phone (
--	phone text,
--	name text,
--	surname text,
--	FOREIGN key (name, surname) references client(name, surname),
--	primary key (phone,name,surname)
--); 





