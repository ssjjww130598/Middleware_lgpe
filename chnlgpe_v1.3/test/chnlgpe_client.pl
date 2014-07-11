#!/usr/bin/perl -w
use strict;
use Socket; 

sub main 
{
	my $port=10003; 

	printf "param1: %s\t", $0;
	printf "param1: %s\t\n", $1;	


	my $host='2.2.2.101';
	my $proto = getprotobyname('tcp');
	my $iaddr = inet_aton($host);
	my $paddr = sockaddr_in($port,$iaddr);

	socket(SOCKET,PF_INET,SOCK_STREAM,$proto) or die "socket:$!";
	connect(SOCKET,$paddr) or die "connect :$!";

	my $cmdStr = "{
		\"Cmd\": \"Start_Stop\",
		\"DeviceModel\": 17,
		\"DeviceID\": 0,
		\"Mode\": \"Start\"
		}
		";
	my $i;
	send(SOCKET, $cmdStr, 0);
	sleep 1;
	close SOCKET or die "close: $!";
}
main();
