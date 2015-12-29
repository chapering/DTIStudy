% -----------------------------------------------------------------------------
% allstlineMedian : given a dir containing a sequence of file each including a 
%				set of streamlines stored in .tgdata (produced by
%				tubegen) format. Simply invoke stlineMedian repeatedly and
%				aggregate all separate result into a single target file, which
%				will hold all median lines in the same format of .tgdata.
%
% Usage:
% @param srcdir name of the source directory
% @param ptNum number of points expected from the interpolation for each
%		streamline
% @param dstfn a file to save the resulting median line, if this is missed, the
%		result will simply be dumped to the stdout
% @return none
%
% Copyright(C) 2010-2011 Chapstudio
% Haipeng Cai, Latitude 23' Longitude 45' May. 8th 2011
% -----------------------------------------------------------------------------
function allstlineMedian(srcdir, ptNum, dstfn)

	format long g;

	if nargin < 1
		error('too few arguments, see the usage by typing doc allstlineMedian...');
		return;
	end

	% interpolate towards 100 points in each streamline by default
	if nargin < 2
		ptNum = 100;
	end

	% read the source directory and list all files, process each with
	% stlineMedian
	try
		allfiles = dir(srcdir);
	catch
		error('Failed to list given directory.');
		return;
	end

	fnnum = length(allfiles);
	reali = 1;
	for i = 1:length(allfiles)
		fn = fullfile(srcdir,allfiles(i).name);
		if isdir(fn)
			fnnum = fnnum - 1;
			continue;
		end

		prompt = sprintf('now processing file %s...\n', fn);
		disp(prompt);

		if nargin >= 3
			if reali == 1
				dlmwrite(dstfn, fnnum);
			end
			dlmwrite(dstfn, ptNum, '-append');
		else
			disp(ptNum);
		end

		res = stlineMedian(fn, ptNum, 'none');

		if nargin >= 3
			dlmwrite(dstfn, res, '-append',...
				'delimiter', ' ', 'precision', '%.6f');
		else
			disp(res);
		end
		reali = reali + 1;
	end

	return;
end

% set ts=4 sts=4 tw=80 sw=4

