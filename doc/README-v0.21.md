## GraphMap - A highly sensitive and accurate mapper for long, error-prone reads 

Preprint of our paper now available on Biorxiv:  
[Fast and sensitive mapping of error-prone nanopore sequencing reads with GraphMap](http://biorxiv.org/content/early/2015/06/10/020719)  

Sequencing data of E. Coli UTI89 generated in-house and used in the paper now available on:  
[PRJEB9557](http://www.ebi.ac.uk/ena/data/view/PRJEB9557)  
  


**__Version: 0.21__**
**Update**
Release date: 02 June 2015  

New alignment mode available: anchored alignment.  

Anchored alignment is an alternative to the default semiglobal alignment. It is less sensitive than default semiglobal, but faster and creates alignments around determined homologies (anchors).  
This is a very powerful addition to alignment, as it creates highly accurate and confident alignments even in the presence of high error rates.  
To run the anchored alignment, use the '-a anchor' option.  

Also, standard Gotoh alignment is also available now as opposed to the default Myers's bit-vector alignment. Custom alignment parameters can be specified via commandline.  
To use the Gotoh alignment, use '-a gotoh' commandline option.  

Additionally, E-value and mapping quality thresholds can now be applied directly from commandline (-z and -c options).  

More to follow.  



**__Version: v0.20b__**
**Update**
Release date: 26 April 2015  

Added the source code.

To build from source:
```
make
```
If the libraries have to be recompiled on your system, type:
```
make deps
```
More installation instructions can be found in the INSTALL file.  


Release date: 02 April 2015  
Precompiled binary, built on Ubuntu 10.04 x64.  
Tested on Mint 17.1 x64.

Significantly improved speed and sensitivity.

Added some important features:
- Mapping quality.
- Sensible alignment score.
- E-value added in reported alignments! Look for a custom ZE parameter in the SAM lines.
- Secondary alignments can now be output as well (use the -Z parameter).

Addressed several reported issues:
- Output only the first whitespace-separated token in the qname field of the SAM output. Previously the entire read's header was output.
- The same for the rname.
- Reads that are marked unmapped now contain no additional mapping information as before.

Please note that by default, GraphMap will use more memory to allow higher speed and sensitivity.
To run in parsimonious (half the memory requirements), please use the -P parameter.

Note #2: some command line parameters were changed (removed/added) since the last version, but most stayed the same.
In case you are using one of the removed parameters, you will be warned and the process will not run.


**__Version: v0.19b__**
Release date: 16 January 2015  
Precompiled binary, built on Ubuntu 10.04 x64.  
Tested on Mint 17.1 x64.  

**Update**
Compiled a MacOS version too, now can also be found in the bin directory.  
Built on MacOS X 10.9.5  

Important updates:
- Better support for circular genomes - use '-C' option if your reference is circular!
- Added a more sensitive mode (though much slower) - check out the '-x' option in the help!
- Better alignments for Illumina reads - again, check out the '-x' option.
- Better dynamic of the AS (alignment score) - value 254 best score, value 0 worst/unmapped.

To use the normal (fast) mode, simply use the default parameters (nothing is changed, just omit the '-x' option).

  
**__Version: v0.18b__**
Release date: 11 December 2014  
Precompiled binary, built on Ubuntu 10.04 x64.  
Tested on Mint 17 (Ubuntu 14.04), Ubuntu Server 14.04, Fedora 20 and Gentoo.

### Description
GraphMap is a novel mapper targeted at aligning long, error-prone third-generation sequencing data.  
It can handle Oxford Nanopore data with very high sensitivity and accuracy, and also presents a significant improvement over the state-of-the-art for PacBio read mappers (namely, compared to BLASR and BWA-MEM).

GraphMap was designed for ease-of-use: the default parameters can handle a wide range of read lengths and error profiles. This is an important feature for technologies where the error rates and error profiles can vary widely across sequencing runs. In addition, GraphMap allows users to uniformly map read datasets from disparate technologies with high sensitivity and accuracy. While GraphMap is not runtime optimized for short-read data (e.g. compared to Bowtie2), it provides accurate and typically more sensitive mappings for Illumina and Ion Torrent reads.

Please keep in mind that this is an early development version and we welcome your comments and feedback on GraphMap.

### Comparison to other mappers

Comparison statistics will be uploaded soon.

### Usage

```
# Process all reads from a given FASTA/FASTQ file with default number of threads:
./graphmap -r escherichia_coli.fa -d reads.fastq -o alignments.sam

# Process reads using more sensitive parameters for Illumina data:
./graphmap -x illumina -r escherichia_coli.fa -d reads.fastq -o alignments.sam

# Process reads from a circular genome:
./graphmap -C -r escherichia_coli.fa -d reads.fastq -o alignments.sam

# Limit the number of threads to 8, and load reads in batches of 50MB:
./graphmap -t 8 -B 50 -r escherichia_coli.fa -d reads.fastq -o alignments.sam

# Process only the first 1000 reads:
./graphmap -B 0 -n 1000 -r escherichia_coli.fa -d reads.fastq -o alignments.sam

# Process all reads from a given folder.
./graphmap -r escherichia_coli.fa -D reads_folder -O alignments_folder

# Generate only the index.
./graphmap -I -r escherichia_coli.fa
```

### Contact information

For additional information, help and bug reports please send an email to one of the following:
ivan.sovic@irb.hr, mile.sikic@fer.hr, nagarajann@gis.a-star.edu.sg
