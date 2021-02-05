CREATE DATABASE Conservatory;

USE Conservatory;

Create TABLE users
(
emailAddress varchar(411) NOT NULL,
loginPassword varchar(411) NOT NULL,
theName varchar(411) Not NULL,
CONSTRAINT UniqueSeeds UNIQUE(emailAddress)
);

Create TABLE userPlant(
emailAddress varchar(411) NOT NULL,
plantName float NOT NULL,
CONSTRAINT seed UNIQUE(plantName)
);

Create TABLE plantTraits(
plantName float NOT NULL,
theTime varChar(411) Not Null,
temperature float NOT NULL,
moisture float NOT NULL,
humidity float NOT NULL,
sunlight float NOT NULL,
CONSTRAINT trait UNIQUE(theTime,plantName)
);
