FROM proxysql/proxysql
RUN apt-get clean && apt-get update --fix-missing
RUN apt-get install -y default-mysql-client
ADD proxysql.cnf /etc/