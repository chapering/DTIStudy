% -----------------------------------------------------------------------------
% stlinespline : given a set of streamlines stored in .tgdata (produced by
%				tubegen) format, interpolate towards a single skeletonic
%				streamline with all these streamlines through the cubic spline
%				fitting
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

	allVertices = [];
	allTotal = 0;
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

		% all streamlines in a bundle have similar colors, we pick that of the
		% first as the color of the final skeletonic line
		if numel(cls) == 0
			cls = Vertices(1,4:6);
		end
		Vertices = Vertices(:,1:3);
		allVertices = [ allVertices; Vertices ];
		allTotal = allTotal + vtTotal;

		% for the next streamline
		curline = fgets(fh);
	end
	fclose(fh);

	% cubic spline interpolation
	nVertices = spline(1:allTotal, allVertices', 1:(allTotal-1)/(lnTotal*ptNum-1):allTotal);
	%nVertices = spline(1:allTotal, allVertices', 1:(allTotal-1)/(ptNum-1):allTotal);

	% --------- for comparison to check the interpolation ------------------
	%
	tVertices = nVertices';
	plot3(Vertices(:,1), Vertices(:,2), Vertices(:,3),'ro-',...
		tVertices(:,1), tVertices(:,2), tVertices(:,3),'b.-');
	%

	% splice the skeletonic line's color to comply with the format of .tgdata
	allcls = ones(ptNum*lnTotal,1)*cls;
	%allcls = ones(ptNum,1)*cls;
	res = [nVertices;allcls']';

	if nargin >= 3
		if dstfn ~= 'none'
			dlmwrite(dstfn, ptNum*lnTotal);
			dlmwrite(dstfn, res,'-append','delimiter',' ','precision','%.6f');
		end
	else 
		disp( ptNum*lnTotal );
		disp( res );
	end

	return;
end

% set ts=4 sts=4 tw=80 sw=4

