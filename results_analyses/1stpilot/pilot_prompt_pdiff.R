###############################################################################
## The statistics of experiment results of DTI density/seeding resolution study
## The program is meant to run by Rscript and dump all results to the stdout ##

## never halt execution
options(error=dump.frames)

########################################################################
cat( sprintf("################   DATA PREPARATION ####################################\n") )
########################################################################
# whole raw dataset
alldata <- read.table('./dataR.txt', header=T, sep=' ')

# see the columns
cat( sprintf("############ Variables of the Raw Data #############\n") )
names(alldata)

# saving variable reference afterwards
attach(alldata)

# take a look to verify if the loading is correct
cat( sprintf("############ The Data as is Read #############\n") )
print(alldata)

cat( sprintf("############ Summary of the Data #############\n") )
summary(alldata)

# remove all participants' first task record which is confused by training&practice time for p1 and p2
data1 <- alldata[! ((Torder%%10) == 1),]

# remove the first task record which is confused by training&practice time, of p1 and p2 only
data2 <-  alldata[! ((Pid==1 | Pid==2 ) & (Torder%%10) == 1), ]

detach(alldata)

# remove task records with wrong answer
correctdata1 <- data1[! (data1$Correct==0), ]
correctdata2 <- data2[! (data2$Correct==0), ]

# Task-specific records
cat( sprintf("############ Task-specific Data #############\n") )
for (i in 1:7) {
	cat ( sprintf(" ######### Data dedicated to Task %d ############\n", i) )
	taskdata <- data1[ data1$Tid==i, ]
	print ( taskdata )
	cat ( sprintf(" ######### Summary of Task %d Data ############\n", i) )
	print( summary ( taskdata ) )
}

# data excluding p3 (who is suspected as a semi-expert)
datanop3 <- data1[ ! (data1$Pid==3), ]

# data from 2nd block only
data2ndblock <- alldata[ (alldata$Torder%%10) > 5, ]

cat( sprintf("############ Data to Analyze #############\n") )
print (data1)

########################################################################
cat( sprintf("################   STATISTICS ANALYSES #################################\n") )
########################################################################
std_err <- function(x) sd(x)/sqrt(length(x))

markcorrelation <- function(x) {
	ret=""
	if (! is.na(x)) {
		if ( x < 0.0 ) {
			ret="Negative relationship"
		} 
		else if ( x < 0.3 ) {
			ret="Weak relationship"
		}
		else if ( x < 0.7 ) {
			ret="Moderate relationship"
		}
		else {
			ret="Strong relationship"
		}
		if ( ret != "" ) {
			ret <- sprintf( "%f\t \"\t ****** %s ****** \t\"\n\n", x,ret)
			cat( ret )
		}
	}
	else {
		ret <- sprintf( "xxxxxx\t \"\t ****** N/A ****** \t\"\n\n")
		cat( ret )
	}
}

marksignificance <- function(x) {
	ret=""
	if (! is.na(x)) {
		if ( x <= 0.05 ) {
			ret="SIGNIFICANT!"
		}
		else if ( x <= 0.06 ) {
			ret="Almsot Significant!"
		}
		if ( ret != "" ) {
			ret <- sprintf( "\"\t ****** %s ****** \t\"\n\n\n", ret)
			cat( ret )
		}
	}
	else {
		ret <- sprintf( "xxxxxx\t \"\t ****** N/A ****** \t\"\n\n")
		cat( ret )
	}
}

########################################################################
cat( sprintf("## Basic statistics on the Task completion time\n") )
attach(data1)

cat( sprintf("# Time mean per participant per tid\n") )
time_meanbreakdown <- tapply(Time, list(Pid,Tid), mean)
print (time_meanbreakdown)

cat( sprintf("# Time standard deviation per participant per tid\n") )
time_sdbreakdown <- tapply(Time, list(Pid,Tid), sd)
print (time_sdbreakdown)

cat( sprintf("# Time mean standard error per participant per tid\n") )
time_msebreakdown <- tapply(Time, list(Pid,Tid), std_err)
print (time_msebreakdown)

########################################################################
cat( sprintf("## Basic statistics on the Task correctness\n") )

cat( sprintf("# Correctness mean per participant per tid\n") )
crt_meanbreakdown <- tapply(Correct, list(Pid,Tid), mean)
print ( crt_meanbreakdown )

cat( sprintf("# Correctness standard deviation per participant per tid\n") )
crt_sdbreakdown <- tapply(Correct, list(Pid,Tid), sd)
print ( crt_sdbreakdown )

cat( sprintf("# Correctness mean standard error per participant per tid\n") )
crt_msebreakdown <- tapply(Correct, list(Pid,Tid), std_err)
print ( crt_msebreakdown )
detach(data1)

########################################################################
cat( sprintf("## Correlation Analyses\n") )

cat( sprintf("\t##overall Time -- Pid\n") )
c <- cor(data1$Time, data1$Pid)
markcorrelation( c )

cat( sprintf("\t##overall Correct -- Pid\n") )
c <- cor(data1$Correct, data1$Pid)
markcorrelation( c )

for (i in 1:7) {
	cat ( sprintf("##========= correlation analysis of Task %d =========\n", i) )
	tdata <- data1 [ data1$Tid == i, ]
	cat (sprintf("\t# Time ~ Pid\n"))
	c <- cor(tdata$Time, tdata$Pid)
	markcorrelation( c )

	cat (sprintf("\t# Correct ~ Pid\n "))
	c <- cor(tdata$Correct, tdata$Pid)
	markcorrelation( c )
}

########################################################################
cat( sprintf("## Distribution functions\n") )
cat( sprintf("# Emperical distribution function of task completion time\n") )
plot.ecdf( data1$Time, main="emperical distribution of task completion time" )

cat( sprintf("# Emperical distribution function of task completion time\n") )
plot.ecdf( data1$Correct, main="emperical distribution of task correctness" )

########################################################################
cat( sprintf("### Two sample T-test of Time between participant for all tasks\n") )
for (n in 1:5) {
	for (m in n:5) {
		if ( m!=n ) {
			cat ( sprintf("#\t Pid %d ---- Pid %d \n", m,n) )
			mdata <- data1[ data1$Pid == m, ]
			ndata <- data1[ data1$Pid == n, ]
			X <- t.test( mdata$Time, ndata$Time, conf.level=0.95, var.equal=F, alternative='t' )
			print (X)
			marksignificance(X$p.value)
		}
	}	
}

########################################################################
cat( sprintf("### Two sample T-test of Correctness between participant for all tasks\n") )
for (n in 1:5) {
	for (m in n:5) {
		if ( m!=n ) {
			cat ( sprintf("#\t Pid %d ---- Pid %d \n", m,n) )
				mdata <- data1 [ data1$Pid == m, ]
				ndata <- data1 [ data1$Pid == n, ]
				X <- t.test( mdata$Correct, ndata$Correct, conf.level=0.95, var.equal=F, alternative='t' )
				print (X)
				marksignificance(X$p.value)
		}
	}	
}

########################################################################
cat( sprintf("### Two sample T-test of Time between participant in each task\n") )
for (i in 1:7) {
	cat( sprintf("## ========= T-test of Task %d =========\n", i) )
	tdata <- data1 [ data1$Tid == i, ]
	for (n in 1:5) {
		for (m in n:5) {
			if ( m!=n ) {
				cat ( sprintf("#\t Pid %d ---- Pid %d \n", m,n) )
				mdata <- tdata [ tdata$Pid == m, ]
				ndata <- tdata [ tdata$Pid == n, ]
				X <- t.test( mdata$Time, ndata$Time, conf.level=0.95, var.equal=F, alternative='t' )
				print (X)
				marksignificance(X$p.value)
			}
		}	
	}
}

########################################################################
cat( sprintf("### Two sample T-test of Correctness between participant in each task\n") )
for (i in 1:7) {
	cat( sprintf("## ========= T-test of Task %d =========\n", i) )
	tdata <- data1 [ data1$Tid == i, ]
	for (n in 1:5) {
		for (m in n:5) {
			if ( m!=n ) {
				cat ( sprintf("#\t Pid %d ---- Pid %d \n", m,n) )
				mdata <- tdata [ tdata$Pid == m, ]
				ndata <- tdata [ tdata$Pid == n, ]
				X <- t.test( mdata$Correct, ndata$Correct, conf.level=0.95, var.equal=F, alternative='t' )
				print (X)
				marksignificance(X$p.value)
			}
		}	
	}
}

########################################################################
cat( sprintf("### AVOVA for all tasks\n") )
cat( sprintf("#\t Time ~ Pid \n") )
boxplot( data1$Time~data1$Pid, main=sprintf("AVOVA of all Tasks - Time~Pid") )
TX <- aov( data1$Time~data1$Pid )
print (TX)
print( summary(TX) )

cat( sprintf("#\t Correct ~ Pid \n") )
boxplot( data1$Correct~data1$Pid, main=sprintf("AVOVA of all Tasks - Correct~Pid") )
CX <- aov( data1$Correct~data1$Pid )
print (CX)
print( summary(CX) )

########################################################################
cat( sprintf("### AVOVA for each task\n") )

for (i in 1:7) {
	cat( sprintf("##========= AVOVA of Task %d =========\n", i) )
	tdata <- data1 [ data1$Tid == i, ]

	cat( sprintf("#\t Time ~ Pid \n") )
	boxplot( tdata$Time~tdata$Pid, main=sprintf("AVOVA of Task %d - Time~Pid", i) )
	TX <- aov( tdata$Time~tdata$Pid )
	print (TX)
	print( summary(TX) )

	cat( sprintf("#\t Correct ~ Pid \n") )
	boxplot( tdata$Correct~tdata$Pid, main=sprintf("AVOVA of Task %d - Correct~Pid", i) )
	CX <- aov( tdata$Correct~tdata$Pid )
	print (CX)
	print( summary(CX) )
}

########################################################################
cat( sprintf("\n## normality checking\n") )
qqnorm( data1$Time,main="normality checking of Time"); qqline( data1$Time)
qqnorm( data1$Correct,main="normality checking of Correct"); qqline( data1$Correct )

########################################################################
cat( sprintf("\n### linear regression model for all tasks\n") )
cat( sprintf("#\t Time ~ Pid \n") )
tfit <- lm(data1$Time~data1$Pid)
print( summary( tfit ) )
par(mfrow=c(2,2)); plot( tfit, main=sprintf("regression model - Time~Pid", i) )

cat( sprintf("#\t Correct ~ Res \n") )
cfit <- lm(data1$Correct~data1$Pid)
print( summary( cfit ) )
par(mfrow=c(2,2)); plot( cfit, main=sprintf("regression model - Correct~Pid", i) )

########################################################################
cat( sprintf("\n### linear regression model for each task\n") )
for (i in 1:7) {
	cat( sprintf("##========= linear regression model of Task %d =========\n", i) )
	tdata <- data1 [ data1$Tid == i, ]

	cat( sprintf("#\t Time ~ Pid \n") )
	tfit <- lm(tdata$Time~tdata$Pid)
	print( summary( tfit ) )
	par(mfrow=c(2,2)); plot( tfit, main=sprintf("regression model of Task %d - Time~Pid", i) )

	cat( sprintf("#\t Correct ~ Res \n") )
	cfit <- lm(tdata$Correct~tdata$Pid)
	print( summary( cfit ) )
	par(mfrow=c(2,2)); plot( cfit, main=sprintf("regression model of Task %d - Correct~Pid", i) )
}

cat( sprintf("\n### The end.\n") )

########################################################################
### Jul. 15~16th 2011 ~ hcai's first R script due to unavailablitiy  ###
### of the proprietary software - SAS								 ###	
########################################################################

