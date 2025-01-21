#!/bin/bash

sudo -u postgres psql postgres -f parameters.sql -f create-db-user.sql -f schema.sql -f grant.sql -f initdb.sql
