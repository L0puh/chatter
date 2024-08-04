CREATE TABLE IF NOT EXISTS users (
  user_id SERIAL PRIMARY KEY, 
  username VARCHAR (50) UNIQUE NOT NULL, 
  password VARCHAR (64) NOT NULL,
  salt VARCHAR (64) NOT NULL
);

CREATE TABLE IF NOT EXISTS posts (
  post_id SERIAL PRIMARY KEY, 
  title VARCHAR (255) NOT NULL, 
  content TEXT NOT NULL
);
