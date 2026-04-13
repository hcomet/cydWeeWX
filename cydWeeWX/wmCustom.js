onload = function() {
    if (document.getElementById('MAXBRIGHTNESSHIDDEN')) {
        document.getElementById('MAXBRIGHTNESSHIDDEN').style.display = 'none';
        document.getElementById('MAXBRIGHTNESS').value = document.getElementById('MAXBRIGHTNESSHIDDEN').value;
        document.getElementById('DIMBRIGHTNESSHIDDEN').style.display = 'none';
        document.getElementById('DIMBRIGHTNESS').value = document.getElementById('DIMBRIGHTNESSHIDDEN').value;
        document.getElementById('DIMMERMODEHIDDEN').style.display = 'none';
        document.getElementById('DIMMERMODE').value = document.getElementById('DIMMERMODEHIDDEN').value;
        document.getElementById('DIMMERSTARTTIMEHIDDEN').style.display = 'none';
        document.getElementById('DIMMERSTARTTIME').value = document.getElementById('DIMMERSTARTTIMEHIDDEN').value;
        document.getElementById('DIMMERENDTIMEHIDDEN').style.display = 'none';
        document.getElementById('DIMMERENDTIME').value = document.getElementById('DIMMERENDTIMEHIDDEN').value;
        document.getElementById('RISESETOFFSETHIDDEN').style.display = 'none';
        document.getElementById('RISESETOFFSET').value = document.getElementById('RISESETOFFSETHIDDEN').value;
        document.getElementById('LDRHIGHTHRESHOLDHIDDEN').style.display = 'none';
        document.getElementById('LDRHIGHTHRESHOLD').value = document.getElementById('LDRHIGHTHRESHOLDHIDDEN').value;
        document.getElementById('LDRLOWTHRESHOLDHIDDEN').style.display = 'none';
        document.getElementById('LDRLOWTHRESHOLD').value = document.getElementById('LDRLOWTHRESHOLDHIDDEN').value;
        document.getElementById('LDRHIGHTHRESHOLD').min = document.getElementById('LDRLOWTHRESHOLD').value;
        document.getElementById('LDRLOWTHRESHOLD').max = document.getElementById('LDRHIGHTHRESHOLD').value;
    }
};
function updateMaxBrightnessField() {
    document.getElementById('MAXBRIGHTNESSHIDDEN').value = document.getElementById('MAXBRIGHTNESS').value;
}
function updateDimBrightnessField() {
    document.getElementById('DIMBRIGHTNESSHIDDEN').value = document.getElementById('DIMBRIGHTNESS').value;
}
function updateDimmerModeField() {
    document.getElementById('DIMMERMODEHIDDEN').value = document.getElementById('DIMMERMODE').value;
}
function updateDimmerStartTimeField() {
    document.getElementById('DIMMERSTARTTIMEHIDDEN').value = document.getElementById('DIMMERSTARTTIME').value;
}
function updateDimmerEndTimeField() {
    document.getElementById('DIMMERENDTIMEHIDDEN').value = document.getElementById('DIMMERENDTIME').value;
}
function updateRiseSetOffsetField() {
    document.getElementById('RISESETOFFSETHIDDEN').value = document.getElementById('RISESETOFFSET').value;
}
function updateLdrHighThresholdField() {
    document.getElementById('LDRHIGHTHRESHOLDHIDDEN').value = document.getElementById('LDRHIGHTHRESHOLD').value;
    document.getElementById('LDRLOWTHRESHOLD').max = document.getElementById('LDRHIGHTHRESHOLD').value;
}
function updateLdrLowThresholdField() {
    document.getElementById('LDRLOWTHRESHOLDHIDDEN').value = document.getElementById('LDRLOWTHRESHOLD').value;
    document.getElementById('LDRHIGHTHRESHOLD').min = document.getElementById('LDRLOWTHRESHOLD').value;
}