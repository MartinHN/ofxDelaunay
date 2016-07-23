/*
 *  ofxDelaunay.h
 *
 *  Created by Pat Long on 29/10/09.
 *  Copyright 2009 Tangible Interaction. All rights reserved.
 *
 *  Some parts based on demo code by Gilles Dumoulin. 
 *  Source: http://local.wasp.uwa.edu.au/~pbourke/papers/triangulate/cpp.zip
 *
 *	Cleaned up by Lukasz Karluk, 19/05/2010
 *
 *  Refactored for 0071 + ofMesh by James George on 21/06/2012
 */

#include "ofxDelaunay.h"

bool ptInTriangle(const ofVec2f & p, const XYZ & p0, const XYZ & p1, const XYZ& p2) {
    float A = 0.5f * (-p1.y * p2.x + p0.y * (-p1.x + p2.x) + p0.x * (p1.y - p2.y) + p1.x * p2.y);
    float sign = A < 0.0f ? -1.0f : 1.0f;
    float s = (p0.y * p2.x - p0.x * p2.y + (p2.y - p0.y) * p.x + (p0.x - p2.x) * p.y) * sign;
    float t = (p0.x * p1.y - p0.y * p1.x + (p0.y - p1.y) * p.x + (p1.x - p0.x) * p.y) * sign;

    return s > 0.0f && t > 0.0f && (s + t) < 2.0f * A * sign;
}

void ofxDelaunay::reset(){
    vertices.clear();
    triangles.clear();
	triangleMesh.clear();
	ntri = 0;
}

int ofxDelaunay::addPoint( const ofPoint& point ){
	return addPoint( point.x, point.y, point.z );
}

int ofxDelaunay::addPoint( float x, float y, float z ){
    XYZI v;
    v.x = x + ofRandom(0.01);
    v.y = y;
    v.z = z;
	v.i = vertices.size();
    vertices.push_back(v);
	return vertices.size();
}


int ofxDelaunay::addPoints( vector<ofPoint>& points ){
	for(int i = 0; i < points.size(); i++){
        addPoint( points[i] );
    }
    return vertices.size();
}

int ofxDelaunay::triangulate(vector<int> *indices ){
    
    if(vertices.size() < 3){return NULL;}
    

	
	// make clone not to destroy vertices
  vector<XYZI> verticesTemp;
  if(indices!=nullptr){
    verticesTemp.resize(indices->size());
    int j = 0;
    for (auto & i:*indices){
      verticesTemp[j] = vertices[i];
      j++;
    }
  }
  else{verticesTemp=vertices;}
	qsort( &verticesTemp[0], verticesTemp.size(), sizeof( XYZI ), XYZICompare );
 int nv = verticesTemp.size();
	//vertices required for Triangulate
    vector<XYZ> verticesXYZ(nv);
	
	//copy XYZIs to XYZ
	for (int i = 0; i < nv; i++) {
		XYZ v;
		verticesXYZ[i].x = verticesTemp.at(i).x;
		verticesXYZ[i].y = verticesTemp.at(i).y;
		verticesXYZ[i].z = verticesTemp.at(i).z;
		
	}
	
    //add 3 emptly slots, required by the Triangulate call
    verticesXYZ.push_back(XYZ());
    verticesXYZ.push_back(XYZ());
    verticesXYZ.push_back(XYZ());
    
    //allocate space for triangle indices
    triangles.resize(3*nv);
    
	Triangulate( nv, &verticesXYZ[0], &triangles[0], ntri );


	//copy triangle data to ofxDelaunayTriangle.
	triangleMesh.clear();
	triangleMesh.setMode(OF_PRIMITIVE_TRIANGLES);
	
    //copy vertices
	for (int i = 0; i < vertices.size(); i++){
        triangleMesh.addVertex(ofVec3f(vertices[i].x,vertices[i].y,vertices[i].z));
    }
	
	//copy triangles
	for(int i = 0; i < ntri; i++){
		triangleMesh.addIndex(verticesTemp.at(triangles[ i ].p1).i);
		triangleMesh.addIndex(verticesTemp.at(triangles[ i ].p2).i);
		triangleMesh.addIndex(verticesTemp.at(triangles[ i ].p3).i);

  }
	return ntri;
}

void ofxDelaunay::draw(){
	if(ofGetStyle().bFill){
	    triangleMesh.draw();
    }
    else{
    	triangleMesh.drawWireframe();
    }

//	for(int i = 0; i < vertices.size(); i++ ){
//		ofDrawBitmapStringHighlight(ofToString(i) , vertices[i].x, vertices[i].y);
//	}
//	ofDrawBitmapStringHighlight("tri: " + ofToString(ntri) + "\nver:" + ofToString(vertices.size()), 30, 300);
}


int ofxDelaunay::getNumTriangles(){
	return ntri;
}


int ofxDelaunay::getNumPoints(){
	return vertices.size();
}

ofPoint ofxDelaunay::getPointNear(ofPoint pos, float minimumDist, int & index){

	XYZI ret;
	XYZ p; p.x = pos.x; p.y = pos.y; p.z = pos.z;
	float minDist = FLT_MAX;
	for(int i = 0; i < vertices.size() ; i++){
		float d = ofDist(vertices[i].x, vertices[i].y, p.x, p.y);
		if(d < minDist ) {
			minDist = d;
			index = i;
			ret = vertices[i];
		}
	}
	if (minDist > minimumDist){
		ret.x = ret.y = ret.z = 0.0f;
		index = -1;
	}
	return ofVec3f(ret.x, ret.y, ret.z);
}

ITRIANGLE ofxDelaunay::getTriangleAtIndex(int index){
	if (index >= 0 && index < ntri){
		return triangles[index];
	}
	return ITRIANGLE();
}

void ofxDelaunay::removePointAtIndex(int index){
	if (index >= 0 && index < vertices.size()){
		vertices.erase(vertices.begin()+index);
	}
	triangulate();
}


void ofxDelaunay::setPointAtIndex(ofPoint p, int index,bool shouldTriangulate){
	if (index >= 0 && index < vertices.size()){
    XYZI pp; pp.x = p.x+ ofRandom(0.01); pp.y = p.y; pp.z = p.z;
    pp.i = index;
		vertices[index] = pp;
	}
	if(shouldTriangulate)triangulate();
}

vector<ofPoint> ofxDelaunay::getPointsForITriangle(ITRIANGLE t){
	vector<ofPoint> pts;
	pts.push_back( ofPoint(vertices[t.p1].x, vertices[t.p1].y, vertices[t.p1].z));
	pts.push_back( ofPoint(vertices[t.p2].x, vertices[t.p2].y, vertices[t.p2].z));
	pts.push_back( ofPoint(vertices[t.p3].x, vertices[t.p3].y, vertices[t.p3].z));
	return pts;
}

ITRIANGLE ofxDelaunay::getTriangleForPos(ofPoint pos){
	
	ITRIANGLE ti;
  ti.p1=0;
  ti.p2=0;
  ti.p3=0;
	for(int i = 0; i < triangleMesh.getIndices().size()/3 ; i++){
    int pp1 = triangleMesh.getIndices()[i*3];
    int pp2 = triangleMesh.getIndices()[i*3+1];
    int pp3 = triangleMesh.getIndices()[i*3+2];

        XYZ p0; p0.x = vertices[pp1].x; p0.y = vertices[pp1].y; p0.z = vertices[pp1].z;
        XYZ p1; p1.x = vertices[pp2].x; p1.y = vertices[pp2].y; p1.z = vertices[pp2].z;
        XYZ p2; p2.x = vertices[pp3].x; p2.y = vertices[pp3].y; p2.z = vertices[pp3].z;
		bool inside = ptInTriangle(pos, p0, p1, p2);
		if(inside) {
      ti.p1 = pp1;
      ti.p2=pp2;
      ti.p3 = pp3;
			break;
		}
	}
	return ti;
}

int XYZICompare(const void *v1, const void *v2){
	XYZI *p1, *p2;
    
	p1 = (XYZI*)v1;
	p2 = (XYZI*)v2;
	if(p1->x < p2->x)
		return(-1);
	else if(p1->x > p2->x)
		return(1);
	else
		return(0);
}
