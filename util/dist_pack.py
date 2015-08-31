#!/usr/bin/python
#
#  Check various C source code policy rules and issue warnings for offenders
#
#  Usage:
#
#    $ python dist_pack.py -z --input-path Dist-files -o TargetFile --out-prefix myProject
#
import os
import sys
import re
import optparse
import zipfile
import tarfile

def write_tar_gz(src, dst, prefix = ''):
	out_file = "%s.tar.gz" % (dst)
	tar_file = tarfile.open(out_file, "w:gz")
	abs_src = os.path.abspath(src)
	for dirname, subdirs, files in os.walk(src):
		for filename in files:
			absname = os.path.abspath(os.path.join(dirname, filename))
			arcname = os.path.join(prefix,absname[len(abs_src) + 1:])
			tar_file.add(absname, arcname)
	print "wrote %s" % (out_file)
	tar_file.close()

def write_tar_bz2(src, dst, prefix = ''):
	out_file = "%s.tar.bz2" % (dst)
	tar_file = tarfile.open(out_file, "w:bz2")
	abs_src = os.path.abspath(src)
	for dirname, subdirs, files in os.walk(src):
		for filename in files:
			absname = os.path.abspath(os.path.join(dirname, filename))
			arcname = os.path.join(prefix,absname[len(abs_src) + 1:])
			tar_file.add(absname, arcname)
	print "wrote %s" % (out_file)
	tar_file.close()

def write_tar_xz(src, dst, prefix = ''):
	out_file = "%s.tar.xz" % (dst)
	tar_file = tarfile.open(out_file, "w:xz")
	abs_src = os.path.abspath(src)
	for dirname, subdirs, files in os.walk(src):
		for filename in files:
			absname = os.path.abspath(os.path.join(dirname, filename))
			arcname = os.path.join(prefix,absname[len(abs_src) + 1:])
			tar_file.add(absname, arcname)
	print "wrote %s" % (out_file)
	tar_file.close()
	
	
def zip(src, dst, prefix = ''):
	zipPath = "%s.zip" % (dst)
	zf = zipfile.ZipFile("%s.zip" % (dst), "w", zipfile.ZIP_DEFLATED)
	abs_src = os.path.abspath(src)
	for dirname, subdirs, files in os.walk(src):
		for filename in files:
			absname = os.path.abspath(os.path.join(dirname, filename))
			arcname = os.path.join(prefix,absname[len(abs_src) + 1:])
			zf.write(absname, arcname)
	print ("wrote %s" % (zipPath))
	zf.close()

def main():
	global problems
	parser = optparse.OptionParser()
	parser.add_option('--tbz2', dest='bz2',action='store_true', default=False, help='Create tar.bz2 archive.')
	parser.add_option('--tgz', dest='gz',action='store_true', default=False, help='Create tar.gz archive.')
	parser.add_option('--txz', dest='xz',action='store_true', default=False, help='Create tar.xz archive. (Requires Python 3.3+)')
	parser.add_option('-z', dest='zip',action='store_true', default=False, help='Create a zip archive.')
	parser.add_option('--input-path', dest='in_path', default='', type='string', 
		action='store', help='specify the Folder to Compress.')
	parser.add_option('--out-prefix', dest='out_prefix', default='', type='string', 
		action='store', help='specify the output Prefix.')
	parser.add_option('-o', dest='out_file', default='', type='string', 
		action='store', help='specify the output Prefix.')
	parser.add_option('--compact-ver', dest='compactVer', default=0, type='int', 
		action='store', help='specify the output Prefix.')		
	(opts, args) = parser.parse_args()
	if(len(opts.in_path) < 1 or len(opts.out_file) < 1):
		print "dist_pack.py: invalid Arguments. Please re-run with -h and try again.";
		exit(-1);
	# Make Sure Python 3.3 is running this script before attempting to run the xz generator.
	if(opts.xz and sys.version_info[:2] < (3, 3)):
		print "dist_pack.py: tar.xz archive format is not available on Python version under v3.3.";
		exit(-1);

	if(not ( opts.zip or opts.bz2 or opts.gz or opts.xz)):
		print "dist_pack.py: no archive type specified. Please re-run with -h and try again.";
		exit(-1);
	prefix = '';
	outfile = opts.out_file;
	if(opts.compactVer):
		ver_major = opts.compactVer / 10000
		ver_minor = opts.compactVer / 100 % 100
		ver_patch = opts.compactVer % 100
		outfile = "%s-%d.%d.%d" % (opts.out_file,ver_major,ver_minor,ver_patch)
		if(opts.out_prefix):
			prefix = "%s-%d.%d.%d" % (opts.out_prefix,ver_major,ver_minor,ver_patch)
		else:
			prefix = outfile
	else:
		if(opts.out_prefix):
			prefix = opts.out_prefix
		else:
			prefix = opts.out_file
	# Create Zip File
	if(opts.zip):
		zip(opts.in_path,outfile,prefix)
	if(opts.bz2):
		write_tar_bz2(opts.in_path,outfile,prefix)
	if(opts.gz):
		write_tar_gz(opts.in_path,outfile,prefix)
	if(opts.xz):
		write_tar_xz(opts.in_path,outfile,prefix)

if __name__ == '__main__':
	main()
