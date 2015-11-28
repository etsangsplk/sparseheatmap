#include <nan.h>
#include <node.h>
#include <stdio.h>
#include "sparsescroll.h"
#include "sparsematrix.h"
#include "sparsearray.h"

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;
using v8::Array;

/**
 * Compile a series of sparse arrays to a single sparse matrix
 */
void CompileCanvas(const Nan::FunctionCallbackInfo <v8::Value> &info) {

  if (info.Length() < 8) {
    Nan::ThrowTypeError("Wrong number of arguments: expected 8.");
    return;
  }

  if (!info[0]->IsNumber() || !info[1]->IsNumber() || !info[2]->IsNumber() || !info[3]->IsArray() ||
      !info[4]->IsNumber() || !info[5]->IsNumber() || !info[6]->IsArray() || !info[7]->IsNumber()) {
    Nan::ThrowTypeError(
            "Wrong arguments: Expected width (Number), height (Number), layout (Number), data (Array), blob width (Number), blob height (Number), blob intensity data (Array), imageWidth (Number).");
    return;
  }

  double width = info[0]->NumberValue();
  double height = info[1]->NumberValue();
  int layout = info[2]->NumberValue();
  v8::Local <v8::Array> dataarr = v8::Local<v8::Array>::Cast(info[3]);
  double blobwidth = info[4]->NumberValue();
  double blobheight = info[5]->NumberValue();
  int destImageWidth = (int) info[7]->NumberValue();
  v8::Local <v8::Array> blobArr = v8::Local<v8::Array>::Cast(info[6]);
  unsigned int *blobVals = new unsigned int[blobArr->Length()];
  // Iterate through the blob array, adding each element to our list
  for (unsigned int i = 0; i < blobArr->Length(); i++) {
    unsigned int intensityval = blobArr->Get(i)->Uint32Value();
    blobVals[i] = intensityval;
  }

  Sparsearray **sparrs = new Sparsearray *[dataarr->Length()];

  // Do the matrix!
  Sparsematrix matrix(width, height, blobwidth, blobheight, layout, blobVals);

  for (unsigned int d = 0; d < dataarr->Length(); d++) {
    Sparsearray *myNewSP = new Sparsearray();
    v8::Local <v8::Object> myObj = v8::Local<v8::Object>::Cast(dataarr->Get(d));
    myNewSP->width = (unsigned int) myObj->GetInternalField(0)->NumberValue();
    myNewSP->height = (unsigned int) myObj->GetInternalField(1)->NumberValue();
    v8::Local <v8::Array> sparseArr = v8::Local<v8::Array>::Cast(myObj->GetInternalField(2));
    myNewSP->datalen = sparseArr->Length();
    myNewSP->data = new unsigned int[sparseArr->Length()];
    for (unsigned int t = 0; t < sparseArr->Length(); t++) {
      unsigned int scoreval = sparseArr->Get(t)->Uint32Value();
      myNewSP->data[t] = scoreval;
    }
    sparrs[d] = myNewSP;

    matrix.integrate_sparsearray(myNewSP);
  }

  //int *finalImageIntensities = matrix.get_intensity_map(destImageWidth);
/*
  Local <Array> v8Array = Nan::New<Array>();
  for (int s = 0; s < matrix.lastIntensitySize; s++) {
    v8Array->Set(s, Nan::New((int) finalImageIntensities[s]));
  }
*/
  Local <Array> v8Array = Nan::New<Array>();
  int dlen = matrix.width * matrix.height;
  for (int s = 0; s < dlen; s++) {
    v8Array->Set(s, Nan::New((int) matrix.data[s]));
  }

  for (unsigned int s = 0; s < dataarr->Length(); s++) {
    delete sparrs[s];
  }
  delete[] sparrs;
  delete[] blobVals;
  //delete[] finalImageIntensities;

  info.GetReturnValue().Set(v8Array);
}

/**
 * Compile a series of sparse arrays to a single sparse matrix
 */
void CompileVScroll(const Nan::FunctionCallbackInfo <v8::Value> &info) {

  if (info.Length() < 2) {
    Nan::ThrowTypeError("Wrong number of arguments: expected 2.");
    return;
  }

  if (!info[0]->IsNumber() || !info[1]->IsArray()) {
    Nan::ThrowTypeError(
            "Wrong arguments: Expected height (Number), data (Array).");
    return;
  }

  double height = info[0]->NumberValue();
  v8::Local <v8::Array> dataarr = v8::Local<v8::Array>::Cast(info[1]);

  Sparsearray **sparrs = new Sparsearray *[dataarr->Length()];

  // Do the matrix!
  Sparsescroll matrix(height);

  for (unsigned int d = 0; d < dataarr->Length(); d++) {
    Sparsearray *myNewSP = new Sparsearray();
    v8::Local <v8::Object> myObj = v8::Local<v8::Object>::Cast(dataarr->Get(d));
    myNewSP->width = (unsigned int) myObj->GetInternalField(0)->NumberValue();
    myNewSP->height = (unsigned int) myObj->GetInternalField(1)->NumberValue();
    v8::Local <v8::Array> sparseArr = v8::Local<v8::Array>::Cast(myObj->GetInternalField(2));
    myNewSP->datalen = sparseArr->Length();
    myNewSP->data = new unsigned int[sparseArr->Length()];
    for (unsigned int t = 0; t < sparseArr->Length(); t++) {
      unsigned int scoreval = sparseArr->Get(t)->Uint32Value();
      myNewSP->data[t] = scoreval;
    }
    sparrs[d] = myNewSP;

    matrix.integrate_sparsearray(myNewSP);
  }

  Local <Array> v8Array = Nan::New<Array>();
  unsigned int matlen = matrix.height;
  for (unsigned int s = 0; s < matlen; s++) {
    v8Array->Set(s, Nan::New((double) matrix.data[s]));
  }

  for (unsigned int s = 0; s < dataarr->Length(); s++) {
    delete sparrs[s];
  }
  delete[] sparrs;

  info.GetReturnValue().Set(v8Array);
}

/**
 * Interface with JavaScript and expose our API
 */
void Init(v8::Local <v8::Object> exports) {
  exports->Set(Nan::New("compile_canvas").ToLocalChecked(),
               Nan::New<v8::FunctionTemplate>(CompileCanvas)->GetFunction());
  exports->Set(Nan::New("compile_vertical_scroll").ToLocalChecked(),
               Nan::New<v8::FunctionTemplate>(CompileVScroll)->GetFunction());
}

// Let node know about this
NODE_MODULE(sparsematrix, Init
)