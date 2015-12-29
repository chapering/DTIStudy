% -----------------------------------------------------------------------------
% stlineMedian : given a set of streamlines stored in .tgdata (produced by
%				tubegen) format, firstly interpolate with each streamline to get
%				equal number of points, then obtain the median of all these
%				streamlines by simplying averaging the interpolated points
%				across them
%
% Usage:
% @param srcfn name of file giving the streamline model
% @param ptNum number of points expected from the interpolation for each
%		streamline
% @param dstfn a file to save the resulting median line, if this is missed, the
%		result will simply be dumped to the stdout
% @return the sequence of point in the median streamline
%
% Copyright(C) 2010-2011 Chapstudio
% Haipeng Cai, Latitude 23' Longitude 45' May. 8th 2011
% -----------------------------------------------------------------------------
function res = stlineMedian(srcfn, ptNum, dstfn)

	format long g;

	if nargin < 1
		error('too few arguments, see the usage by typing doc stlineMedian...');
		return;
	end

	% interpolate towards 100 points in each streamline by default
	if nargin < 2
		ptNum = 100;
	end

	% read streamlines and store geometry for each single streamline into a
	% matrix, while all matrices are stashed in a cell array
	try
		fh = fopen(srcfn);
	catch
		error('Failed to open source file.');
		return;
	end

	cls = [];

	vtSum = zeros(3,ptNum);
	curline = fgets(fh);
	lnTotal = str2num(curline);
	curline = fgets(fh);
	while ~feof(fh) && ischar(curline)
		vtTotal = str2num(curline);

		Vertices =[];
		for vtCnt = 1:vtTotal
			if feof(fh)
				error('error encountered during file parsing...');
				return;
			end
			curline = fgets(fh);
			Vertices = [Vertices; str2num(curline)];
		end

		cls = Vertices(1,4:6);
		Vertices = Vertices(:,1:3);

		% cubic spline interpolation
		nVertices = spline(1:vtTotal, Vertices', 1:(vtTotal-1)/(ptNum-1):vtTotal);

		% --------- for comparison to check the interpolation ------------------
		%{
		tVertices = nVertices';
		plot3(Vertices(:,1), Vertices(:,2), Vertices(:,3),'r.-',...
			tVertices(:,1), tVertices(:,2), tVertices(:,3),'b.-');
		%}

		vtSum = vtSum + nVertices;

		% for the next streamline
		curline = fgets(fh);
	end
	fclose(fh);

	pres = vtSum'*1.0/lnTotal;

	% append casual color to comply with the format of .tgdata
	%allcls = ones(ptNum,3);
	allcls = ones(ptNum,1)*cls;

	res = [pres';allcls']';

	if nargin >= 3
		if dstfn ~= 'none'
			dlmwrite(dstfn, ptNum);
			dlmwrite(dstfn, res,'-append','delimiter',' ','precision','%.6f');
		end
	else 
		disp( ptNum );
		disp( res );
	end

	return;
end

% set ts=4 sts=4 tw=80 sw=4

