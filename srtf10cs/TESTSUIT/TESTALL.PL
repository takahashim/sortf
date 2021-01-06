#! /usr/local/bin/perl

# do all the tests for ported `sortf'
# required subdirectories
#	./script 	: resides test scripts
# 	./data 		: input data for those scripts
#	./should	: output shoud be like these

# mtoyo@Lit.hokudai.ac.jp
# $Header: /home/mtoyo/sortf/testsuit/testall.pl,v 1.3 1994/08/28 00:29:43 mtoyo Exp mtoyo $

$tm_start= time;

require 'getopts.pl';
&Getopts("f");	# -f to make authenticated data in `./should'
$force_write_authentic = 1 if (defined($opt_f));
$dir_script= 	'script';
$dir_data= 		'data';
$dir_authentic= 'should';
$tmpout=		'TESTOUT';
$path= (-e '/dev/null')? '/' : '\\'; 
@kcode_list= ('euc', 'sjs', 'non');
$current_kcode=	&giveme_kcode(@kcode_list);
$s_kcode_list= join('|',@kcode_list);

opendir(SCRIPT,$dir_script) 	|| die "cannot find script directory `$dir_script' here\n";
(-e $dir_data) 		|| die "cannot find data directory `$dir_data' here\n";
(-e $dir_authentic) || die "cannot find authentic output directory `$dir_authentic' here\n";

@fname_dir= grep(!/^\.\.?$/,readdir(SCRIPT));

while(@fname_dir) {
	$n_test++;
	$fname= $fname_full= shift(@fname_dir); 
	if ($fname =~ /\.($s_kcode_list)/o) {
		$kcode_tail= $1; $fname= $`;
		next if ($kcode_tail ne $current_kcode);
	}
	$script_name= 		$dir_script.$path.$fname_full;
	$data_name= 		$dir_data.$path.$fname;
	$authentic_name= 	$dir_authentic.$path.$fname;
	if (-e $script_name) {
		if ($command= &get_command($script_name)) {
			print "$fname : ";
				# 1. do sort
			if ((-e $data_name) && ($force_write_authentic || (-e $authentic_name))) {
				unlink($tmpout);
				$outfname= ($force_write_authentic)? $authentic_name : $tmpout;
				$execstr= $command . " -o$outfname " . $data_name;
				print "$execstr\n";
				$execstr =~ s/\\/\\\\/g;
				$v= system("$execstr");
				# 2. do compare
				if (!$force_write_authentic && &do_compare($tmpout,$authentic_name)) {
					print "NG for test $fname\n";
					push(@NG_cases,$fname);
				} else {
					$n_OK++;
				}
			} else {
				warn "cannot find $data_name / $authentic_name\n";
				push(@NG_files,$fname);
			}
			unlink($tmpout);
		}
	}
}	

$n_errors= scalar(@NG_cases) + 0;
$n_file_errors= scalar(@NG_files) + 0;
$tm_used= time - $tm_start;
if ($force_write_authentic) {
	print STDERR "$n_test authentic results created, $n_file_errors files not found, $tm_used seconds.\n";
} else {
	print STDERR "$n_test tests, of which $n_OK tests OK, $n_errors errors, $n_file_errors files not found, $tm_used seconds.\n";
}

sub get_command {
	local($fname)= @_;
	if (!open(FIN,$fname)) {
		warn "cannot open `$fname'\n"; return '';
	}
	while(<FIN>) {
		$_ =~ s/\s*$//;				# cut traling spaces
		$_ =~ s/^\s*//;				# cut leftest spaces
		next if ($_ =~ /^#/); 		# ignore comments
		next if ($_ =~ /^\s*$/);	# ignore NULL lines
		close(FIN);
		return $_;
	}
	return '';
}

sub do_compare {
	local($file_1, $file_2)= @_;
	local(@s1, @s2);
	
	return 1 if (!open(F1,$file_1) || !open(F2,$file_2));
	
	@s1= <F1>; close(F1);
	@s2= <F2>; close(F2);
	
	while(@s1 && @s2) {
		$rec1= shift(@s1);
		$rec2= shift(@s2);
		return 1 if ($rec1 ne $rec2) ;
	}
	return 0;
}	

sub giveme_kcode {
	local($euc,$sjis,$none)= @_;
	local($byte1,$byte2)= unpack("CC",'‚O');
	return $euc  if (($byte1 == 0xa3) && ($byte2 == 0xb0));
	return $sjis if (($byte1 == 0x82) && ($byte2 == 0x4f));
	return $none;
}

