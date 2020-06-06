sshpass -p "root" scp -P 10023 system.conf root@localhost:/etc/system.conf
sshpass -p "root" ssh root@localhost -p 10023 'rm -rf /usr/src/minix/drivers/dfa'
sshpass -p "root" scp -P 10023 -r dfa root@localhost:/usr/src/minix/drivers/dfa
sshpass -p "root" scp -P 10023 ioc_dfa.h root@localhost:/usr/include/sys/ioc_dfa.h
sshpass -p "root" scp -P 10023 ioc_dfa.h root@localhost:/usr/src/minix/include/sys/ioc_dfa.h
sshpass -p "root" scp -P 10023 ini root@localhost:/ini
sshpass -p "root" scp -P 10023 ioc_example.c root@localhost:/ioc_example.c
sshpass -p "root" scp -P 10023 ioc_reject.c root@localhost:/ioc_reject.c
sshpass -p "root" scp -P 10023 ioc_reset.c root@localhost:/ioc_reset.c
sshpass -p "root" scp -P 10023 ioc_round.c root@localhost:/ioc_round.c
sshpass -p "root" scp -P 10023 go_round.c root@localhost:/go_round.c