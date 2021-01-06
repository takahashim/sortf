#! /usr/local/bin/jperl
# mkjptbl.pl  : make colljp.c initial table
# (c) mtoyo@Lit.hokudai.ac.jp, 1994
# $Header: RCS/mkjptbl.pl 1.17 94/05/14 14:22:02 mtoyo Exp $
#
# ================================================================
#  Nota bene
# ----------------------------------------------------------------
# This file contains Multi-byted Japanese Kanji characters.
# o It can be executed only by Multi-byte-sensitive `Perl'
# o It may cause an unwanted effect on non Multi-byte-sensitive
#   displays / printers.
#
 
 
require 'getopts.pl';
&Getopts('J:');
$must_escape_backslash= $opt_J if ((defined $opt_J) && $opt_J);
 
$out_fname= 'jptbl.h';
if (-f $out_fname) {
    $backup_fname= $out_fname . "BK";
    warn "moving old `$out_fname' to `$backup_fname'\n";
    unlink($backup_fname) if (-f $backup_fname);
    unless (rename($out_fname,$backup_fname)) {
        die "could not move `$out_fname' to `$backup_fname' : quit saving old `$out_fname'.\n";
    }
}
open(FOUT,">$out_fname") || die "cannot open `$out_fname'\n";
 
($sec,$min,$hour,$mday,$mon,$year)= localtime(time);
print FOUT "\n#if 0\n";
print FOUT join("\n\t",'',$out_fname,'made by mkjptbl.pl',' $Header: RCS/mkjptbl.pl 1.17 94/05/14 14:22:02 mtoyo Exp $', "generated on $year/$mon/$mday $hour:$min:$sec");
print FOUT "\n\n#endif\n";
 
$s_HIRAGANA=        # �������̃��X�g    $s_KATAKANA �ƑΉ������鎖(�����̃�����������)
    '������������������������������������������������������������'.
    '�����������ÂĂłƂǂȂɂʂ˂̂͂΂ςЂт҂ӂԂՂւׂ؂قڂ�'.
    '�܂݂ނ߂��������������������T�U';
$s_KATAKANA=        # �Љ����̃��X�g    $s_HIRAGANA �ƑΉ������鎖(�����̃�����������)
    '�@�A�B�C�D�E�F�G�H�I�J�K�L�M�N�O�P�Q�R�S�T�U�V�W�X�Y�Z�[�\�]'.
    '�^�_�`�a�b�c�d�e�f�g�h�i�j�k�l�m�n�o�p�q�r�s�t�u�v�w�x�y�z�{�|'.
    '�}�~�����������������������������������������R�S������';
$s_KANA_skelton=    # �����𐴉��ɁA��������啶���ɁA��������Љ����ɒu������������
    '�A�A�C�C�E�E�G�G�I�I�J�J�L�L�N�N�P�P�R�R�T�T�V�V�X�X�Z�Z�\�\'.
    '�^�^�`�`�c�c�c�e�e�g�g�i�j�k�l�m�n�n�n�q�q�q�t�t�t�w�w�w�z�z�z'.
    '�}�~�����������������������������������������T�U�E�J�P';
$s_vowel=           # �ꉹ���ɒu������������
    '�A�A�C�C�E�E�G�G�I�I�A�A�C�C�E�E�G�G�I�I�A�A�C�C�E�E�G�G�I�I'.
    '�A�A�C�C�E�E�E�G�G�I�I�A�C�E�G�I�A�A�A�C�C�C�E�E�E�G�G�G�I�I�I'.
    '�A�C�E�G�I�A�A�E�E�I�I�A�C�E�G�I�A�A�C�G�I���T�U�E�A�G';
$s_DAKUON=          # �������̃��X�g
    '�K�M�O�Q�S�U�W�Y�[�]�_�a�d�f�h�o�r�u�x�{���������������������������Âłǂ΂тԂׂځS�U';
$s_HANDAKUON=       # ���������̃��X�g
    '�ς҂Ղ؂ۃp�s�v�y�|';
$s_small=           # �������̃��X�g
    '�����傟����������b�������@�B�D�F�H������';
$s_ODORI=           # �����p�J��Ԃ��L��(�x�莚)�̃��X�g
    '�R�T�S�U';
$s_CHOUON=          # �����L��(����)�̃��X�g
    '�[';
$s_1KANA=           # 1�o�C�g���� (�g���ȁI ^^;)
    '';
$s_2KANA= $s_HIRAGANA . $s_KATAKANA;
 
 
$s_2space=          # 2 �o�C�g�A�Ԋu
    '�@';
$s_1space=          # 1 �o�C�g�A�Ԋu
    ' ';
$s_2punct=          # 2 �o�C�g�A��؂�L��
    '�A�B�C�D�E�F�G�H�I�Q�\�]�^�_�`�a�b�c�d';
$s_1punct=          # 1 �o�C�g�A��؂�L��(2 �o�C�g�ƑΉ������鎖�A�������̂�1�o�C�g�󔒂�)
    '  ,. :;?!_  /�_~ |  ';
$s_2paren=          # 2 �o�C�g�A���ʋL��
    '�e�f�g�h�i�j�k�l�m�n�o�p�q�r�s�t�u�v�w�x�y�z';
$s_1paren=          # 1 �o�C�g�A���ʋL��(2 �o�C�g�ƑΉ������鎖�A�������̂�1�o�C�g�󔒂�)
    '`\' "()  []{}';
$s_2math=           # 2 �o�C�g�A�w�p�L��
    '�{�|�}�~�����������������������ځہ܁݁ށ߁���������聸���������������ȁɁʁˁ́́�';
$s_1math=           # 1 �o�C�g�A�w�p�L��(2 �o�C�g�ƑΉ������鎖�A�������̂�1�o�C�g�󔒂�)
    '+-   = <>                             ^      ';
$s_2common=         # 2 �o�C�g�A��ʋL��
    '���������������������������������������������������������';
$s_1common=         # 1 �o�C�g�A��ʋL��(2 �o�C�g�ƑΉ������鎖�A�������̂�1�o�C�g�󔒂�)
    '#&*@��';
$s_2unit=           # 2 �o�C�g�A�P�ʋL��
    '����������������������';
$s_1unit=           # 1 �o�C�g�A�P�ʋL��(2 �o�C�g�ƑΉ������鎖�A�������̂�1�o�C�g�󔒂�)
    '    \\$  %';
$s_2alpha=          # 2 �o�C�g�A�����L�� (�啶���������̑Ή��͎��Ȃ�)
    '�������ÃăŃƃǃȃɃʃ˃̃̓΃σЃу҃ӃԃՃ�'.
    '������������������������������������������������' .
    '�p�q�r�s�t�u�v�w�x�y�z�{�|�}�~������������������������������������'.
    '�@�A�B�C�D�E�F�G�H�I�J�K�L�M�N�O�P�Q�R�S�T�U�V�W�X�Y�Z�[�\�]�^�_�`';
$s_1alpha=          # 1 �o�C�g�A�����L��(����Ȃ��̖������� :-)
    '';
$s_1latin_lower=            # 1 �o�C�g�A���e���A���t�@�x�b�g������
    'abcdefghijklmnopqrstuvwxyz';
$s_2latin_lower=            # 2 �o�C�g�A���e���A���t�@�x�b�g������
    '����������������������������������������������������';
$s_1latin_upper=            # 1 �o�C�g�A���e���A���t�@�x�b�g�啶��
    'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
$s_2latin_upper=            # 2 �o�C�g�A���e���A���t�@�x�b�g�啶��
    '�`�a�b�c�d�e�f�g�h�i�j�k�l�m�n�o�p�q�r�s�t�u�v�w�x�y';
$s_1latin=          # 1 �o�C�g�A���e���A���t�@�x�b�g
    $s_1latin_lower . $s_1latin_upper;
$s_2latin=          # 2 �o�C�g�A���e���A���t�@�x�b�g
    $s_2latin_lower . $s_2latin_upper;
$s_1digit=          # 1 �o�C�g����
    '0123456789';
$s_2digit=          # 2 �o�C�g����
    '�O�P�Q�R�S�T�U�V�W�X';
$s_GETA=            # �����L��  (�u�����p�ہv�͕����ƔF�߂Ȃ�����O��)
    '��';
$s_KANJI=           # ����(�{���� ISO10646/JIS2122���ɑS�����X�g����΂�����ł���...)
    '�V�W�X�Y�Z';
 
@flag_name= ('FL_DAKUON','FL_HANDAKUON','FL_ODORI','FL_CHOUON','FL_KATAKANA','FL_small','FL_lower','FL_upper');
$n= 1;
foreach $fl (@flag_name) {
    eval("\$$fl= $n");
    $n*= 2;
}
 
$n_genre= 0;
$genre_order{'GR_SPACE'}= ++$n_genre;
$genre_order{'GR_PUNCT'}= ++$n_genre;
$genre_order{'GR_PAREN'}= ++$n_genre;
$genre_order{'GR_MATH'}= ++$n_genre;
$genre_order{'GR_COMMON'}= ++$n_genre;
$genre_order{'GR_UNIT'}= ++$n_genre;
$genre_order{'GR_ALPHA'}= ++$n_genre;
$genre_order{'GR_DIGIT'}= ++$n_genre;
$genre_order{'GR_LATIN'}= ++$n_genre;
$genre_order{'GR_KANA'}= ++$n_genre;
$genre_order{'GR_KANJI'}= ++$n_genre;
$genre_order{'GR_GETA'}= ++$n_genre;
 
&set_skelton($s_2space.$s_1space,   $s_2space.$s_2space,'');
&set_skelton($s_2punct.$s_1punct,   $s_2punct.$s_2punct,'');
&set_skelton($s_2paren.$s_1paren,   $s_2paren.$s_2paren,'');
&set_skelton($s_2math.$s_1math,     $s_2math.$s_2math,'');
&set_skelton($s_2common.$s_1common, $s_2common.$s_2common,'');
&set_skelton($s_2unit.$s_1unit,     $s_2unit.$s_2unit,'');
&set_skelton($s_1alpha.$s_2alpha,   $s_1alpha.$s_2alpha,'');
&set_skelton($s_1latin_lower,       $s_2latin_upper,'');
&set_skelton($s_2latin_lower,       $s_2latin_upper,'');
&set_skelton($s_1latin_upper,       $s_2latin_upper,'');
&set_skelton($s_2latin_upper,       $s_2latin_upper,'');
&set_skelton($s_2digit.$s_1digit,   $s_2digit.$s_2digit,'');
&set_skelton($s_GETA,               $s_GETA,'');
&set_skelton($s_KANJI,              $s_KANJI,'');
&set_skelton($s_HIRAGANA,           $s_KANA_skelton,$s_vowel);
&set_skelton($s_KATAKANA,           $s_KANA_skelton,$s_vowel);
&set_skelton($s_CHOUON,'','');
 
&set_genre($s_1space .  $s_2space,  'GR_SPACE');
&set_genre($s_1punct .  $s_2punct,  'GR_PUNCT');
&set_genre($s_1paren .  $s_2paren,  'GR_PAREN');
&set_genre($s_1math  .  $s_2math,   'GR_MATH');
&set_genre($s_1common.  $s_2common, 'GR_COMMON');
&set_genre($s_1unit.    $s_2unit,   'GR_UNIT');
&set_genre($s_1alpha.   $s_2alpha,  'GR_ALPHA');
&set_genre($s_1latin.   $s_2latin,  'GR_LATIN');
&set_genre($s_1digit.   $s_2digit,  'GR_DIGIT');
&set_genre($s_1KANA.    $s_2KANA . $s_CHOUON,   'GR_KANA');
&set_genre($s_KANJI,                'GR_KANJI');
&set_genre($s_GETA,                 'GR_GETA');
 
&set_flag($s_1latin_lower . $s_2latin_lower,$FL_lower);
&set_flag($s_1latin_upper . $s_2latin_upper,$FL_upper);
&set_flag($s_DAKUON,$FL_DAKUON);
&set_flag($s_HANDAKUON,$FL_HANDAKUON);
&set_flag($s_small,$FL_small);
&set_flag($s_ODORI,$FL_ODORI);
&set_flag($s_CHOUON,$FL_CHOUON);
&set_flag($s_KATAKANA,$FL_KATAKANA);
 
$DQ= '"';
 
print FOUT "#if 0\n";
print FOUT  "\t,{ source, skelton,  vowel, flag, \tlength(stub), genre }\n";
print FOUT "#endif\n\n\n";
 
# if memory is tight : comment out this line
    @sorted_key= sort by_genre keys(%tbl_skelton);
# and uncomment lines below instead
#   warn "(info) output is not sorted due to tight memory\n";
#   @sorted_key= keys(%tbl_skelton);    # if memory is tight
 
while(@sorted_key) {
    $original= shift(@sorted_key);
    $converted= $tbl_skelton{$original};
    if ($original  =~ /[\"\\]/) { $original=    '\\' . $original; }
    if ($converted =~ /[\"\\]/) { $converted=   '\\' . $converted; }
    $cv_vowel= $tbl_vowel{$original};
    if ($must_escape_backslash) {
        $original=  &escape_backslash($original);
        $converted= &escape_backslash($converted);
        $cv_vowel=  &escape_backslash($cv_vowel);
    }
    $name_genre= $tbl_genre{$original};
    printf(FOUT "\t%1s{ %-6s%-6s%-6s\t",
            $mid_comma,
            $DQ . $original . $DQ . ',' ,
            $DQ . $converted. $DQ . ',' ,
            $DQ . $cv_vowel . $DQ . ',' );
    $flags= $tbl_flag{$original};
    undef(@flag_valid);
    foreach $fl (@flag_name) {
        eval("push(\@flag_valid,'$fl') if (\$flags & \$$fl)");
    }
    printf FOUT "%-30s\t(0),$name_genre }\n", (((@flag_valid)? (join('|',@flag_valid)) : '0') . ',');
    $mid_comma= ',';
    $n_lines++;
}
close(FOUT);
print STDERR "output $n_lines items to `$out_fname'\n";
 
exit;
 
sub by_genre {
    local($diff);
    return $diff if ($diff = ($genre_order{$tbl_genre{$a}} <=>  $genre_order{$tbl_genre{$b}}));
    return $diff if ($diff = ($tbl_skelton{$a} cmp  $tbl_skelton{$b}));
    $a cmp $b;
}
 
sub set_flag {
    local($s,$flag)= @_;
    local(@c)= split(//,$s);
    local($all_space)= (($s =~ /^[\s�@]+$/)?1 : 0);
    local($cc);
    while(@c) {
        $cc= shift(@c);
        $tbl_flag{$cc} |= $flag if (($cc !~ /^ $/) || $all_space);
        warn "($cc) undefined skelton\n" if (!defined($tbl_skelton{$cc}));
    }
}
 
sub set_genre {
    local($s,$genre)= @_;
    local(@c)= split(//,$s);
    local($all_space)= (($s =~ /^[\s�@]+$/)?1 : 0);
    local($cc);
    while(@c) {
        $cc= shift(@c);
        if (($cc !~ /^ $/) || $all_space) {
            if (defined ($old_genre= $tbl_genre{$cc})) {
                print FOUT "'$cc'\t genre($genre) already defined old genre($old_genre)\n" if ($old_genre ne $genre);
            }
            $tbl_genre{$cc} = $genre;
        }
        warn "($cc) undefined skelton\n" if (!defined($tbl_skelton{$cc}));
    }
}
 
sub set_skelton {
    local($s_from, $s_skel, $s_vow)= @_;
    local(@c)= split(//,$s_from);
    local(@a_skelton)= split(//,$s_skel);
    local(@a_vowel)= split(//,$s_vow);
    local($all_space)= ((($s_from =~ /^[\s�@]+$/) && ($s_skel =~ /^[\s�@]+$/))?1 : 0);
    local($cc, $skel, $vowel);
    while(@c) {
        $cc= shift(@c);
        $skel=shift(@a_skelton);
        $vowel=shift(@a_vowel);
        $tbl_skelton{$cc}= $skel    if ((($skel !~ /^ $/) && ($cc !~ /^ $/)) || $all_space);
        $tbl_vowel{$cc}= $vowel     if ((($vowel !~ /^ $/)&& ($cc !~ /^ $/)) || $all_space);
    }
}
 
sub escape_backslash {
    local($s)= @_;
    local(@out_string);
    local($backslash)= unpack("C",'\\');
    local($c, $byte1, $byte2);
    foreach $c (split(//,$s)) {
        if (length($c) > 1) {
            ($byte1, $byte2)= unpack("C*",$c);
            $c .= '\\' if ($byte2 ==  $backslash);
        }
        push (@out_string,$c);
    }
    return join('',@out_string);
}