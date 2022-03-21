It has been too long since the release of version 1.10 in Dec 10 2008.
Below is a complete description of the work since then.  The files in this
release are:

teem-1.11.0-src.tar.gz    : The complete Teem source tree
NrrdIO-1.11.0-src.tar.gz  : The source tree for NrrdIO, a minimal library
                            for doing IO of NRRD files (a subset of Teem)

========== Coding and Testing

teem/src/Coding.txt is now an updated description of the basic coding
coding conventions and rational that characterize Teem

The teem/Testing directory has a growing number of tests.  Thanks to James
Bigler there is now teem/testDataPath.h.in which provides a means for tests to
load in test datasets from the teem/data directory.

"TEEM_LIB_LIST" is now near places in code, makefiles, and any other file
were all the Teem libraries are listed or named.  This helps make sure that
all libraries are listed in the same consistent dependency order, and to
identify what needs changing when libraries are added or removed.

========== CMake building =========

This is more mature now, thanks to work of David Cole at Kitware.  For
example, he (quoting commit message) fixed problems when building applications
on the Mac that try to use the Teem built into Slicer: Compute Teem_ROOT_DIR
based on the detected location of TeemConfig.cmake and make all path
references to files in the installed TeemConfig.cmake relative to
Teem_ROOT_DIR.

========== Python wrappers ==========

teem/python and teem/python/ctypes are new directories;
teem/python/ctypes includes "teem-gen.py" (thanks to Sam Quinan),
which automatically generates "teem.py", a complete ctypes wrapper for
all of Teem using ctypeslib (based on gccxml).

========== command-line binaries ==========

CHANGE: Removed affine, pos2idx, and idx2pos.  The exact same functionality is
available in unu affine (that is, you can actually give it five constants, no
nrrds necessary), unu w2i, and unu i2w, respectively.  Goal is to prune the
total number of Teem binaries.

NEW: Added a program called "gprobe" (general probe) which subsumes the
functionality of vprobe and pprobe.  It allows you to probe on any grid of any
dimension, or a point, or the original grid.  Eventually gprobe will be
multi-threaded.  gprobe also supports (copied from pprobe) using discrete
differences to measure derivatives of scalar quantities, in order to validate
the items for the analytic derivatives

NEW: miter now handles axis data that encodes the scaling in the
spaceDirection vector

NEW: unu commands or functionality:
* setting the UNRRDU_QUIET_QUIT environment variable (just setting it,
  or setting it to anything) now prevents a chain of unu commands from
  generating a huge list of documentation for all the commands.
* unu diff (nrrdCompare)
* unu fft (nrrdFFT)
* unu i2w and w2i new (see above)
* unu axinfo: can now set per-axis kind with unu axinfo -k, and
  can now set information for multiple axes by giving multiple
  arguments to -a
* unu affine (nrrdArithAffine and nrrdArithIterAffine)
* unu quantize, unu histo, and unu histax: the -min and -max arguments now
  accept things of the form "x%", where the "%" suffix indicates that the
  bound is specified in terms of a percentile.  The percentiles are determined
  with a histogram, the size of which in "unu quantize" can be set with "-hb"
* unu acrop (nrrdCropAuto)
* unu sselect (nrrdSliceSelect)
* unu 1op expm1 (nrrdUnaryOpExpm1)
* unu 2op -w and unu 3op -w for explicit selection of which argument
  determines output shape.  Prior to this, the output shape was always
  determined entirely by the first non-constant nrrd
  (nrrdArithIterBinaryOpSelect and nrrdArithIterTernaryOpSelect)
* unu 2op rrand (nrrdBinaryOpRicianRand)
* unu 3op gauss (nrrdTernaryOpGaussian)
* unu 3op rician (nrrdTernaryOpRician)
* unu jhisto can take a single input and compute joint histogram of
  slices along some axis (for example RGB color channels). This
  removes the need to do a separate prior "unu dice". The functionality
  may later migrate down to nrrdHistoJoint (by Benjamin Trofatter)
* unu slice and unu project can usefully act on 1-D input.
* unu resample's "-s" option now excepts "/<float>" for dividing number of
  samples by something, "+=<uint>" and "-=<uint>" for adding or
  subtracting some number of samples, as well as "a" for resampling
  that preserves the aspect-ratio of the resampled axes
* unu cksum (nrrdCRC32)
Also, the end of the intro paragraph in the unu command usage/docs now
identifies the underlying nrrd library function call that implements
the command

========== Changes with libraries  ==========

Michel Audette made many code changes across Teem (no API changes) to fix
Win64 warnings, and to bring Nrrd up to date with the NrrdIO in ITK, and
then Gordon Kindlmann revisited these in 2012 to resynch again, prior
to the 1.11 release.

API NEW: All libraries have a const int named "<libname>Present"
(e.g. airPresent, hestPresent, biffPresent, ...).  These can be used to
programmatically asses which libraries are part of a given
libteem shared library.  Can also look at meetTeemLibs.

---------- meet ----------

This is a new library, which by design depends on all the other libraries, and
thereby can serve as a common point (one could say a meeting point) for
accessing things uniformly through-out Teem, regardless of what library they
are in.  For example:

* meetAirEnumAll() allocates and populates an NULL-terminated array of all
the airEnums in Teem (manually maintained).  meetAirEnumAllPrint() prints
this array to a file.

* meetNrrdKernelAll() allocates and returns a NULL-terminated array of all
the NrrdKernels in Teem

* meetHestGageKind() is a uniform way of parsing gageKind's from the
command-line.  Now used by pprobe, vprobe, gprobe, deconv, and mrender.

* meetAirEnumAllCheck() runs airEnumCheck() on all airEnums

* meetTeemLibs is an argv-style (NULL-terminated array of char*) list of all
the Teem libraries in this build

* meetNrrdKernelAllCheck() runs nrrdKernelCheck on all kernels, and
also numerically tests that nrrdKernelBlahD is in fact the derivative
of nrrdKernelBlah.

---------- elf, tijk ----------

These are new libraries, created by Thomas Schultz to support visualization
(elf) and computation (tijk) of high-order tensors and spherical harmonics.
Elf depends on Tijk and will be the counterpart of ten for higher-order DW-MRI
modeling (if you're wondering about the name, look up "elf" in a German
dictionary ;-) Elf includes functions that implement the core of the recent
EuroVis10 paper by Schultz and Kindlmann ("A Maximum Enhancing Higher-Order
Tensor Glyph")

---------- air ----------

API CHANGE: removed AIR_32BIT and airMy32Bit from air.h, which had allowed
Teem-using libraries to learn if they are on a 32-bit or 64-bit platform.
Now, nothing in the Teem code is specific to this distinction, so learning it
has been removed from the build process, and it is no longer available via
AIR_32BIT or airMy32Bit.  Also, airInsane_32Bit has been removed as one of the
possible returns of airSanity().

API CHANGE, but not really:
* removed _AIR_SIZE_T_CNV and _AIR_PTRDIFF_T_CNV, which were never intended to
be in the public API anyway; these are the %-prefixed conversion or formatting
sequences for size_t and ptrdiff_t.  Unfortunately these are
platform-specific, and maintaining them on all platforms got to be too
annoying.  So now its done via the new airSprintSize_t and airSprintPtrdiff_t
and a tmp buffer
* airMopAdd now returns 0. This facilitates its use in a sequence of calls
being error-checked by looking for non-zero return values.

API CHANGE: airEqvSettle, and hence airEqvMap, no longer return the maximum
class ID after settling, they return the total number of different IDs

API CHANGE: removed airEnumLast since it was never used in Teem

API CHANGE: more removal of functions that aren't used or used only once in
hest: airBinaryPrintUInt, airILoad, airFLoad, airDLoad, airIStore, airFStore,
airDStore.  airIStore and airDLoad were created for hest, which is the only
place they were used, now they have been moved to hest as static functions

API CHANGE: (but actually not anything you would notice): Large patch for
airEnum type. Use const char * instead of fixed length strings for the various
airEnum components. This results in smaller library sizes and is more thread
safe as those strings can be moved to the immutable read only parts of the
teem library.

API NEW: added airPtrPtrUnion which is a union of various pointers-to
pointers, which exists solely for quieting compiler warnings about aliasing
(often on the first argument to airArrayNew)

API NEW: added airVanDerCorput() and airHalton() for Van Der Corput and Halton
sequences, and a list of prime numbers in airPrimeList[].

API NEW: airSprintSize_t and airSprintPtrdiff_t, for printing values of type
size_t and ptrdif_t.

API NEW: added airPrettySprintSize_t for printing a size_t value in a
human-readable form, e.g. "3.3 MB" or "99 bytes"

API NEW: added airSprintVecSize_t for printing a vector of size_t
into a given (pre-allocated) string

API NEW: added airStrcmp, which is like strcmp but safe to call on NULL
strings

API NEW: added airHeap, an implementation of a binary min-heap, built on top
of airArray (by Thomas Schultz)

API NEW: added airCrc32, for computing 32-bit CRCs on data, same what
cksum does (made available via nrrdCRC32 and unu cksum)

API NEW: Added airEnumCheck, a long-overdue function that checks on the
construction and internal consistency of an airEnum. Given how tricky it can
be to manually maintain these once they take up more than one page, this
function is needed.  Many problems, some many years old, were fixed with this.

API NEW: airRician and airLogRician computes Rician distribution function (and
its log)

API NEW: added AIR_ROUNDUP_UI and AIR_ROUNDDOWN_UI as unsigned int versions of
rounding macros

API NEW: added airEnumPrint to print contents of an airEnum. This has helped
debug problems in the (compile-time) creation of the airEnums associated with
gageKinds, many of which had long-standing errors.

API NEW: added airRandMTStateGlobalInit() so that airRandMTStateGlobal could
be used outside the context of the various random functions (like airDrandMT()
or airSrandMT()) that normally allocate and initialize it as part of being
called

API NEW: added airStderr, airStdout, airStdin to give uniform access to the
respective FILE *s from higher-level language wrappings around Teem (such as
from python/ctypes, which doesn't automatically give you an easy way to refer
to stdout/stderr/stdin)

Bug fixes:
 - airOneLine can now deal with legacy Mac text files, for which the line
   termination is only a \r carraige return
 - airStrcpy no longer has buffer over-runs.
 - After some hand-wringing, removed run-time tests on signaling NaN
(SNAN) from air/sane.c, since its too easy for the creation of an SNAN to
generate warnings/errors that look like a real bug.  From r5258 commit:
    As of July 4 2012 GLK decides that the signaling NaN tests are
    more trouble than they're worth: the signal-ness of the NaN is not
    preserved in double-float conversion for some platforms (so
    airFP_SNAN == airFPClass_d(AIR_SNAN) has never been enforced), and
    there are more platforms for which (apparently) passing AIR_SNAN to
    airFPClass_d changes it to a quiet NaN, which defeats the purpose
    of the test.  To summarize, given that:
    ** AIR_NAN and AIR_QNAN are checked here to be quiet NaN, after
       casting to both float and double,
    ** quiet NaN "hi bit" is tested above, and that
    ** quiet and signaling NaN are mutually exclusive,
    skipping the signaling NaN tests is unlikely to undermine knowing
    the correctness of the compile-time representation of NaNs.  So the
    following line is now commented out for all platforms.

Other:
 - airSingleSscanf now recognizes "pi" as AIR_PI (adding to its special
 handling of "inf" and "nan")

---------- hest ----------
Thanks to Bill Martin and Dave Weinstein, hest should now handle command-line
options that contain spaces.  This had long been an annoyance with hest-based
programs.  For example,

  unu make -t "unsigned short"

wouldn't work before, nor would

  unu slice -i Picture\ 1.png      or    unu save -o Picture\ N.png

But now they do work.

---------- biff ----------

These changes are thanks to Michael Callahan, and represent the most
systematic changes made in Teem to date.

API CHANGE: BIFF_STRLEN is gone.  Because biffAddf exists (see next item),
there is no reason for sprintf'ing into a local buffer, and then passing that
to biffAdd.  The BIFF_STRLEN #define had existed previously as a convenience
for this buffer length.

API NEW: added biffMsgErrNum to learn (without looking inside biffMsg) number
of error messages saved in a biffMsg

API NEW: created the biffAddf function. This is biffAdd except that it takes
in a format string and variable args, alleviating the need to build the error
string before calling biffAdd.

API NEW: created biffMovef and biffMaybeAddf functions to match the biffAddf
function. Note that biffMovef requires the parameters to be reordered when
moving from biffMove so that the error string is last.

---------- hest ----------

API CHANGE: added some long-overdue const-correctness to hest;
argv and some other char* should be const.  Specifically:
-HEST_EXPORT int hestParse(hestOpt *opt, int argc, char **argv,
+HEST_EXPORT int hestParse(hestOpt *opt, int argc, const char **argv,
                           char **errP, hestParm *parm);
-HEST_EXPORT void hestParseOrDie(hestOpt *opt, int argc, char **argv,
+HEST_EXPORT void hestParseOrDie(hestOpt *opt, int argc, const char **argv,
                                 hestParm *parm,
-                                char *me, char *info,
+                                const char *me, const char *info,

API NEW: added hestParm->noArgsIsNoProblem, which makes it *not* an error for
a hest-based program to be invoked with no command-line args, and to instead
set all values entirely from given defaults.  This removes an annoying quirk
in hest usage, whereby one of the command-line options had to be singled out
as having no default value, even when a sensible default value exists, because
running the program with no options would trigger printing usage info.  Now
that you can use "--help" to get usage info, it should become more common to
have programs that don't require any options and can be invoked without any.

---------- nrrd ----------

API CHANGE/NEW: made the NrrdKernelSpec* arg to nrrdKernelSpecSprint const,
and added nrrdKernelSprint so that you can make a string representation of
kernel/kparm pair, without having to create a NrrdKernelSpec

API CHANGE: renaming nrrdKernelHermiteFlag to nrrdKernelHermiteScaleSpaceFlag

API NEW: added nrrdRangePercentileSet and nrrdRangePercentileFromStringSet.
These functions contain all the smarts previously located in unrrdu/quantize.c
which handles range specifications in terms of percentiles of value
distribution.

API NEW: added nrrdArithIterBinaryOpSelect and nrrdArithIterTernaryOpSelect
which are just like nrrdArithIterBinaryOp and nrrdArithIterTernaryOp, except
that now you can explicitly specify which of the inputs is to be used as the
template for the output array shape and size.  The logic with the old
functions is that the first non-constant-valued nrrdIter determines the output
shape, but this is limiting and led to some weird unu contortions. These are
available via the "-w" options to "unu 2op" and "unu 3op"

API NEW: nrrdCompare, nrrdAxisInfoCompare, and nrrdArrayCompare (available via
"unu diff"). This was motivated by the desire to do more CTest tests, which
depend on seeing if two given nrrds are really the same.  nrrdAxisInfoCompare
compares fields in the NrrdAxisInfo structs, and nrrdArrayCompare compares
values in an array.  nrrdCompare compares all fields in two nrrds.

API NEW: added nrrdKernelCompare for determining two pairs of kernel/parms.
This actually looks at the NrrdKernel pointer values, rather than comparing
the results of evaluting the kernel methods.

API NEW: added nrrdKernelCheck to make sure that a kernel is
self-consistent.  Checks things like the kernel's reported support and
integral, and making sure the various ways of evaluating the kernel
all agree.

API NEW: nrrdFFT as wrapper around new FFTW inclusion.  Uses
fftw_plan_guru_dft so that you can compute the transform of any subset of the
axes of a nrrd, which is not possible with fftw_plan_many_dft.  Accessible via
"unu fft".

API NEW: added nrrdCRC32 to call airCrc32 on nrrd->data (available via
unu cksum)

API NEW: added nrrdCropAuto. For doing cropping of a nrrd around some central
portion that is interesting.  Projects slices down to a 1-D array of slice
measurements, and measures cumulative sum of array starting from below and and
from above, finding the cropping bounds in terms of some fraction of the total
sum. Accessible via "unu acrop".

API NEW: nrrdSliceSelect is effectively a combination of nrrdSlice and
nrrdJoin, allowing one to create an output that is a collection of slices from
some input (usable with unu sselect)

API NEW: added nrrdArithAffine and nrrdArithIterAffine (accessible via unu
affine), providing whole nrrd access to the AIR_AFFINE macro.

API NEW: nrrdLoadMulti for reading a sequence of nrrds from similarly-named
files, via a sprintf format string

API NEW: NrrdResampleContext now has overrideCenter field, to be set via
nrrdResampleOverrideCenterSet (available via unu resample -co).  The effect is
to *over-ride* existing per-axis centering information, and use something
else.  The existing NrrdResampleContext->center field (set via unu resample
-c) was only to assert a centering when an input axis did know its centering.

API NEW: NrrdResampleContext has a new nonExistent field, to hold value from
new airEnum nrrdResampleContextNonExistent, to give more control over how
resampling treats non-existent values (available via unu resample -ne).  Old
behavior, and still the default behavior (as per
nrrdDefaultResampleNonExistent) is a noop (nrrdResampleNonExistentNoop): let
the non-existent values pollute the convolution sum.  Now, you can ignore
non-existent values in two ways: nrrdResampleNonExistentRenormalize:
renormalize weights of existent vals, nrrdResampleNonExistentWeight: just use
weights of existent vals as is.

API NEW: added nrrdSpacingStatus airEnum to help describe the possible
nrrdSpacingStatus* enum values

API NEW: nrrdUnaryOpExpm1, accessible via "unu 1op expm1", which is the
inverse of nrrdUnaryOpLog1p (unu 1op log1p)

API NEW: nrrdBinaryOpRicianRand is a binary op for adding Rician noise
(available via unu 2op rrand)

API NEW: nrrdTernaryOpGaussian and nrrdTernaryOpRician for evaluating Gaussian
and Rician distributions; also available as "unu 3op gauss" and "unu 3op
rician"

API NEW: nrrdTernaryOpMaxSmooth and nrrdTernaryOpMinSmooth. These are like
nrrdBinaryOpMax and nrrdBinaryOpMin, but the transition between the linear
ramp and the constant value is smoothed by using erf(), where you control the
width of the transition.  Available via "unu 3op max_sm" and "unu 3op min_sm"

API NEW: nrrdTernaryOpLTSmooth and nrrdTernaryOpGTSmooth. These are like
nrrdBinaryOpLT and nrrdBinaryOpGT, but the transition between 0 and 1 is
smoothed by using erf(), where you control the width of the transition.
Available via "unu 3op lt_sm" and "unu 3op gt_sm".

API NEW: nrrdBoundaryMirror is a new behavior for dealing with samples outside
the range of known samples, it implements mirroring.  Accessible via "unu
resample -b mirror" and "unu pad -b mirror".  Like the other boundary methods,
this isn't available in gage yet (gage only knows the equivalent of
nrrdBoundaryBleed)

API NEW: added some parameter-less kernels for Bsplines and their
first, second, and third derivatives:
cubic: nrrdKernelBSpline3, nrrdKernelBSpline3D, nrrdKernelBSpline3DD,
       nrrdKernelBSpline3DDD (even though this is piece-wise constant)
quintic: nrrdKernelBSpline5, nrrdKernelBSpline5D, nrrdKernelBSpline5DD,
         nrrdKernelBSpline5DDD
septic: nrrdKernelBSpline7, nrrdKernelBSpline7D, nrrdKernelBSpline7DD
        nrrdKernelBSpline7DDD
Also added third derivative of c4hexic, nrrdKernelC4HexicDDD

API NEW: added nrrdKernelBSpline3ApproxInverse,
nrrdKernelBSpline5ApproxInverse, nrrdKernelBSpline7ApproxInverse, and
nrrdKernelC4HexicApproxInverse: Approximate numerical (but quite accurate)
pre-filtering (inverse) discrete kernels that allow non-interpolating kernels
nrrdKernelBSpline3, nrrdKernelBSpline5, nrrdKernelBSpline7, and
nrrdKernelC4Hexic to interpolate.  There are better ways of doing these,
especially for Bsplines (see Unser et al B-Spline Signal Processing IEEE TSP
41(2)), but until that is implemented, these work fine, and trivially fit
within the existing kernel framework

API NEW: added nrrdKernelCatmullRom ("ctmr") and nrrdKernelCatmullRomD
("ctmrd") kernels, which are parameter-less versions of "cubic:0,0.5" and
"cubicd:0,0.5"

API NEW: added nrrdKernelC5Septic (and nrrdKernelC5SepticD,
nrrdKernelC5SepticDD, nrrdKernelC5SepticDDD), an 8-sample support piecewise
septic (7th-order) polynomial which is C^5 continuous, and which accurately
reconstructs up to quartic polynomials (errors show up when reconstructing
quintics).  This is thus the most "accurate" kernel in Teem in the Taylor
series sense.

API NEW: added nrrdKernelBoxSupportDebug and nrrdKernelCos4SupportDebug,
kernels with fixed shape of box and cos(pi*x)^4, respectively, with real
support from -0.5 to 0.5, but with variable nominal support for exercising
functions (like nrrd's resampling or gage's probing) that depend on kernel
support. Also have nrrdKernelCos4SupportDebugD, nrrdKernelCos4SupportDebugDD,
and nrrdKernelCos4SupportDebugDDD.

API NEW: nrrdZeroSet zeros out the values in a nrrd

API NEW: added nrrdMeasureNormalizedL2 and nrrdMeasureRootMeanSquare,
available with "unu project -m nls" and "unu project -m rms".  With
nrrdMeasureNormalizedL2 the division by N (the number of values) is
outside the square root; with nrrdMeasureRootMeanSquare, its inside.

API NEW: added nrrdMeasureCoV for coefficient of variation, available
with "unu project -m cov"

API NEW: nrrdProject and nrrdSlice (and unu project and unu slice) can
now usefully act on 1-D arrays.  In these cases a new axis 1 is
inserted prior to acting on them, which internalizes the "unu axinsert
-a 1" idiom that had been the way of dealing with this in unu.

BUG FIX: Fixed long-standing bug in nrrdShuffle: if the axis being shuffled
had more than NRRD_DIM_MAX samples, nrrdShuffle was apt to write to the
buff1[] string out of bounds as it documented (for the content string) how the
samples were ordered. This was due to a cut-and-paste error using code from
nrrdAxesPermute and nrrdReshape. Now, the permutation is documented only if
the axis is shorter than some size, currently fixed at
LONGEST_INTERESTING_AXIS==42

BUG FIX: OS bug work-around from Dave Weinstein: There's a bug in fread/fwrite
in gcc 4.2.1 (with OSX SnowLeopard). When it reads/writes a >=2GB data array,
it pretends to succeed (i.e. the return value is the right number) but it
hasn't actually read/written the data. The work-around is to loop over the
data, reading/writing 1GB (or smaller) chunks.

---------- ell ----------

API NEW: added ell_3v_area_spherical_d, for computing area of spherical
triangle spanned by given three vectors. Havent fully decided between
requiring that vecs are normalized, versus normalizing them internally

API NEW: added ell_3v_barycentric_spherical_d for barycentric coordinates (in
triangles only) on the unit sphere

API NEW: added ELL_4V_LERP_TT, ELL_2V_SET_TT, ELL_2V_COPY_TT, like all the
other _TT macros it has a cast built-in.  Someday all the AIR_CASTs and
_TTs need to be scrutinized.

API NEW: added ell_4v_norm_f, and there are probably more like it coming

API NEW: ELL_3MV_CONTR2, ELL_4V_ZERO_SET, ELL_2MV_MUL, ELL_2M_ROTATE_SET,
ELL_9V_SUB, ELL_9V_LEN

---------- gage ----------

API CHANGE, sort of: the gageScl3PFilter_t function, which is the
work-horse of how convolution is done to measure things, has a
different API in order to gracefully support different derivatives.
It is unlikely that this would be noticed outside Teem, so this is
probably a non-issue.  Its part of the gage public API because other
gageKinds (within Teem) use it.  This change was made in anticipation
of gage supporting higher-order derivatives.

API CHANGE(s), sort of: This is an API change in the sense that gage will work
the same on most volumes, but will fail with error messages on some other
volumes.  gage is now less permissive and less flexible about the types of
volumes that it can work on: either per-axis spacings or space directions have
to be set. All centerings, if known, have to be the same. This change created
the oppportunity to basically rewrite gageShapeSet() and clean up its
logic. Thus:

* The gageParmDefaultSpacing, gageParmRequireAllSpacings and
gageParmRequireEqualCenters values of the gageParm* enum have been removed,
and so can't be used as arguments to gageParmSet()

* The gageParm->defaultSpacing, gageParm->requireEqualCenters, and
gageParm->requireAllSpacings fields have been removed from the gageParm
struct.

* The globals gageDefDefaultSpacing, gageDefRequireAllSpacings, and
gageDefRequireEqualCenters are gone.

The new behavior should be equivalent to both requireEqualCenters and
requireAllSpacings being non-zero (true).

Other API changes peripherally related to this:

* The gageShape->volHalflen and gageShape->voxLen fields have been removed
from gageShape; these were only set and meaningful in the context of when gage
implicitly placed the volume in a bi-unit cube. Nothin in Teem (outside gage)
used gageShape->volHalflen, and though something did use gageShape->voxLen, it
was set the exact same values as gageShape->spacing, so anything using
gageShape->voxLen can use gageShape->spacing instead.

* void _gageShapeUnitItoW() in shape.c has been changed to static void
shapeUnitItoW(), since nothing outside that source file needs to know about
it.

API NEW: added gageParmGenerateErrStr: a way of controlling if detailed errors
are sprintfd to gage->errStr upon error (can slow things down)

API CHANGE/NEW in some stack-related behavior. Now there is control over
normalizing the weights in the reconstruction (of iv3s) across the stack
(before this was always done, w/out any control), and, there is control over
whether derivative measures are normalized stack position (scale). So:

rename gageParmStackRenormalize --> gageParmStackNormalizeDeriv
rename gageContext->stackRenormalize --> gageContext->stackNormalizeDeriv
rename gageDefStackRenormalize --> gageDefStackNormalizeDeriv
added gageParmStackNormalizeRecon
added gageContext->stackNormalizeRecon
added gageDefStackNormalizeRecon

API CHANGE (but probably no one notices) gageStackBlur now takes gageKind
instead of baseDim, and reports differences (in checking mode)
accordingly. Also, gageStackBlur can now double as a checker on an existing
array of pre-blurred volumes, at the cost of losing an opportunity for const
correctness

API CHANGE (unlikely to be noticed outside Teem): gagePoint's internal fields
are no longer xi, yi, zi, xf, yf, zf; its been replaced by idx[4] and
frac[4]. Internal to gage, this info is now properly used (fixing a bug!) to
determine when the iv3s have to be refilled

API CHANGE: NIXED globals gageErrStr and gageErrNum, which should have been
removed a long time ago, with the intro of per-context errStr and errNum

API NEW: two gageItems of interest to flow field vis were added (by Thomas
Schultz) to the gageKindVec: gageVecStrain and gageVecSOmega

API NEW: added gageItemPack, which stores a set of items related by
 differentiation.

API NEW: added orientationFromSpacing to gageParm, and to the gageShape. When
enabled, gage respects the per-axis spacing settings, and uses that
information to implicitly make an axis-aligned orientation definition, even if
no full orientation info (as with space origin and space directions) is
available. In pprobe/vprobe/gprobe, it is available via the -ofs command line
flag.

API NEW: the gageContext now has a new output field "double edgeFrac" which
records, for the latest probe, the fraction of samples in the iv3 for which
values had to be invented (currently always by bleeding) because the support
of the kernels didn't fit completely inside the volume. This allows gage users
to easily detect (without dealing with index space computations) whether the
last probe location was at the edge of the volume.

API NEW: gageStackVolumeGet which is first pass at function for facilitating
caching of pre-blurred volumes for scale-space, encapsulating logic that had
previously been handled individually by vprobe, main-pull, etc added some
commented-out debugging printfs to gageShapeSet

API NEW: ADDED gageErrStackSearch, a new error for failing to find the index
position of a world-space stack probe position

API NEW: added gageOptimalSigmaSet, to improve location of samples along sigma
for Hermite-spline-based reconstruction of scale-space.  This smarts is
available in vprobe and gprobe (and for some reason no pprobe) via "-sso"

API NEW: added gageStackWtoI and gageStackItoW for doing world <--> index
coordinate conversions along stack,

---------- limn ----------

API NEW: Added limnPolyDataCompress to prune unindexed vertices (By Thomas
Schultz)

API NEW: limnPolyDataJoin for concatenate any number of limnPolyDatas (By
Thomas Schultz)

API NEW: added limnPolyDataSpiralBetterquadric, made
limnPolyDataSpiralSuperquadric be a wrapper around that

API NEW: added limnPolyDataInfoTang for surface tangent aligned with the
gradient of the first texture coordinate

API NEW: limnPolyDataOctahedron, limnPolyDataCubeTriangles, limnPolyDataSquare

API NEW: Added limnPolyDataNeighborList and limnPolyDataNeighborArray to find
all vertex neighbors in a limnPolyData struct. (by Thomas Schultz)

API NEW: limnPolyDataIcoSphere approximates the unit sphere by (repeatedly)
subdividing the icosahedron. (by Thomas Schultz)

API NEW: limnPolyDataSmoothHC: Simple method for mesh smoothing, implementing
Vollmer et al., Improved Laplacian Smoothing of Noisy * Surface Meshes,
Eurographics/CGF 18(3), 1999 (by Thomas Schultz)

API NEW: limnPolyDataVertexNormalsNO: per-vertex normal generation for
non-orientable surfaces (by Thomas Schultz)

API NEW: Added limnPolyDataClipMulti to clip according to multiple values (to
preserve a vertex, all thresholds have to be met). In contrast to the previous
limnPolyDataClip, this actually clips triangles on the boundary.  Turned
limnPolyDataClip into a wrapper to inherit this new functionality. (by Thomas
Schultz)

Fixed memory leak in limnPolyDataSpiralTubeWrap

limnPolyDataVertexNormals now supports triangle strips and fans For
unsupported types, an error is added to biff and 1 is returned

---------- seek ----------

API NEW/CHANGE: (Thanks to Thomas Schultz) Integrated code that implements
Schultz/Theisel/Seidel: "Crease surfaces: From theory to extraction and
application to diffusion tensor MRI." IEEE TVCG 2009.

Main changes to the API:
- Introduced seekType{Ridge,Valley}SurfaceT that implements the above paper
(T stands for the transformed Hessian which is used in that approach)
- Introduced seekType{Ridge,Valley}SurfaceOP that implements marching cubes
based extraction with outer products for local orientations (for comparison)
- Introduced seekItemHessSet (only required for T-based extraction; needs to
be a gageItem that measures the Hessian in the scalar field)
- Introduced seekEvalDiffThreshSet (only for T-based extraction; reasonable
values are on the order of the strength parameter)
- Added seekVertexStrength to simplify proper post-filtering of creases
- Removed strengthMin (no longer needed)

Implementation of marching cubes no longer has topology problems, and may have
some other benefits, thanks to use of MC tables from the macet tool by Carlos
Dietrich, described in, Dietrich et al. Edge Groups: an approach to
understanding the mesh quality of marching methods. IEEE TVCG 2008 (Thanks to
Carlos Dietrich and Thomas Schultz)

---------- ten ----------

API NEW: added tenGlyphBqd* functions to support T Schultz, GL Kindlmann.
Superquadric Glyphs for Symmetric Second-Order Tensors. IEEE TVCG Nov/Dec
2010, 16(6):1595-1604

API NEW: added tenGageFiberCurving and tenGageFiberDispersion items to
tenGageKind, to support P. Savadjiev, G. Kindlmann, S. Bouix, M. E. Shenton,
C.-F. Westin Neuroimage Volume 49, pg 3175-3186 2010

API NEW:
int tenFiberProbeItemSet(tenFiberContext *tfx, int item);
int tenFiberMultiProbeVals(tenFiberContext *tfx,
                           Nrrd *nval, tenFiberMulti *tfml);
Adds ability to set (via tenFiberProbeItemSet) a gage item that will be probed
and saved at every vertex of a computed tractography path.  The probed values
can be learned en masse via tenFiberMultiProbeVals.  This addition accompanied
a significant change in the internal implementation of fiber tracking, but
existing functionality should be all the same.

API NEW: added two more items to tenGage: tenGageFARidgeLineAlignment and
tenGageFARidgeSurfaceAlignment for looking at the alignment between
eigenvectors of the Hessian of FA, and of the tensor itself

API NEW: the tenModel struct is a way of describing multi-parameter models of
the DWI signal, including single tensors, as well as other things.  Still very
much under construction, API in flux

API NEW: added tenRotateSingle_f for rotating a single tensor

BUG FIX (or rather a 99% fix):
ten/experSpec.c/tenDWMRIKeyValueFromExperSpecSet() had been saving DWI
gradient directions to KVPs in the NRRD header with only "%g %g %g"
instead of "%.17g %.17g %.17g" (the analog of which has been used for
the rest of the floating-point nrrd meta-data for quite awhile).  This
caused needless significant precision loss in how the gradients were
saved. However, there is a still a subtle (almost negligible) change,
caused by the normalization done at the end of
ten/chan.c/tenDWMRIKeyValueParse().  The problem is that the computed
norm of what should be a unit length vector is not *exactly*
unit-length, so the normalization, which should sometimes be a no-op,
actually changes the values very slightly.  Fixing this will require
more thought or an API change.

---------- mite/miter/mrender ----------

Possible behavior change: Because of changes to gage above, miter and mrender
can no longer render volumes that don't have any spacing or orientation info,
as they could before.

---------- pull ----------

The "pull" library actually existed in release 1.10, but never quite
worked.  Much of the Teem development since the last release has been
for this library, and Raul San Jose Estepar has done a good chunk of
this.  Now the pull library is the basis of:

GL Kindlmann, R San Jose, SM Smith, C-F Westin.  Sampling and
Visualizing Creases with Scale-Space Particles.  IEEE Trans. on
Visualization and Computer Graphics, 15(6):1415-1424, 2009.
