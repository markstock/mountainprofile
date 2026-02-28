#!/usr/bin/perl
#
# makeallprofiles.pl - look through lidar directories and collect mountain profile images
#
# (c)2026 Mark J. Stock

use File::Basename;

my $rootdir = "/media/nas/mstock/tinymtn/lidar";
#my $rootdir = "/home/mstock/tinymtn/lidar";
my $outdir = "./mtnfigs";
my $mtnbin = "/home/mstock/code/mountainplots/mountainplots.bin";

if (! -d "$outdir") { mkdir $outdir; }
open(my $cmdfile, '>>', "commands") or die "Can't open commands: $!";
print $cmdfile "\n";

my @dirs = glob("${rootdir}/*");

# check each entry in the rootdir - we only want directories
foreach my $dir (@dirs) {
  #print "testing (${dir})\n";
  if (-d "${dir}") {
    #print "  is a directory\n";
    # save the dir name
    my $outbase = basename($dir);
    # now look for text files that contain "elevs" (workdir is optional)
    my @files = glob("${dir}/*");
    foreach my $file (@files) {
      if (-f $file && -T $file && -s $file < 2048) {
        my $filebase = basename($file);
        # this screws it up
        if ($filebase ne "commands" && $filebase ne "HOWTO") {
        #print "  ($filebase) is a text file\n";
        # now, if that file contains "elevs" then we can proceed
        open(my $infile, '<', $file) or die "Can't open $file: $!";
        my ($elevs) = grep { /elevs/ } <$infile>;
        if ($elevs) {
          print "\n${outbase} has elevs in file ${filebase}\n";

          # save black and white elevations
          my @toks = split(' ',$elevs);
          print "  tokens ($toks[1]) and ($toks[2])\n";
          my $minelev = $toks[1];
          my $maxelev = $toks[2];

          # skip out if it's too short
          if ($maxelev - $minelev > 650) {

          # pull and save resolution
          seek($infile, 0, 0);
          my ($res) = grep { /masterres/ } <$infile>;
          @toks = split(' ',$res);
          my $dsmres = $toks[1];
          if (! $dsmres) { $dsmres = 0.635; }
          #print "  found masterres: (${dsmres})\n";

          # grep and save workdir (if not "work")
          seek($infile, 0, 0);
          my ($workline) = grep { /workdir/ } <$infile>;
          @toks = split(' ',$workline);
          my $workdir = $toks[1];
          if (! $workdir) { $workdir = "work"; }
          #print "  setting work dir: (${workdir})\n";

          # do we have enough information?
          # check for existence of master.png in workdir
          my $srcimg = "${dir}/${workdir}/master.png";
          my $outname = "${outdir}/${outbase}.png";
          #print "  checking for (${res})\n";
          if (-f $srcimg && ! -f $outname) {
            #print "  making ${outname}\n";

            my $command = $mtnbin;
            $command .= " -i ${srcimg}";
            $command .= " -o ${outname}";
            $command .= " --elevs ${minelev} ${maxelev}";
            $command .= " --mpp ${dsmres}";
            print "  running (${command})\n";
            system $command;
            print $cmdfile "${command}\n";

            # if mpp is anything other than 1.0, resize
            if (int(${dsmres}) != 1) {
              $command = "pngtopam ${outname} | pamscale ${dsmres} | pamtopng > temp.png";
              system $command;
              print $cmdfile "${command}\n";
              rename "temp.png", ${outname};
            }
          }
          }
        }
        }
      }
    }
  }
}
close $cmdfile


