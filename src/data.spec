struct majMin {
  int major;  /* major version number */
  int minor;  /* minor version number */
};

tag VERSION {
  majMin atrigVersion;
  majMin atrDmpVersion;
  majMin t2scVersion;
  majMin t2trVersion;
  majMin t2caVersion;
};


struct calGeomType {
  int     calo;
  int     caloModule;
  float   RorZmin;
  int     nPhi;              /* phiMin = 0. for all calorimeters */
  int     nEta;
  float   etaMin; 
  int     idEta[nEta];
  int     idEta2[nEta];      /* tile has a 2nd set of subcells */
};

tag CALGEOM {
  int nGeom;
  calGeomType calGeom[nGeom];
};

struct trtGeomType {
  int Part;    /* Part no. */
  int Plane;   /* Plane no. */
  float rMin;
  float rMax;
  float Zmin;
  float Zmax;
  float Phi0;
  int Nstraw;  /* No. Straws in this Plane */
};

tag TRTGEOM { 
  int nGeom;
  trtGeomType table[nGeom];
}; 

struct sctGeomType {
  int address;    /* Packed module address */
  float r;        /* Radius of Module Plane */
  float z;        /* z of Module Plane */
  int nphi;       /* no. Modules in Phi */
  float phi0;
  float dphi;
  float phiOff;
  float tilt;
  float stagger;
  float sas1;
  float sas2;
  int nStrips;
  int n2;
  float pitch;
  float pitch2;
  float stripLen;
};

tag SCTGEOM {
  int nGeom;
  sctGeomType table[nGeom];
};



tag EVHEAD {
  int runNo;
  int eventNo;
};


tag ROIHEAD {
   int runNo;
   int eventNo;
   int roiId;
   int objType;
   int fakeRoi;
   float PhiMin;
   float PhiMax;
   float EtaMin;
   float EtaMax;
   float zVertMin;
   float zVertMax;
   int t1Et;
   int t1EIsol;
   int t1HIsol;
   int t1EmMsk;
};

struct kine {
  int id;
  int idPar;
  int nPar;
  int idBrem;
  int nBrem;
  int idProd;
  int nProd;
  int idBro;
  int iPart;
  float pt;
  int q;
  float eta;
  float phi0;
  float d0;
  float z0;
  float phic;
  float etac;
  float rv;
  float zv;
  float rdec;
  float zdec;
} ;

tag KINE {
 int nKine;
 kine kineList[nKine];
};

struct emCalDigiType {
  int id;
  float Et;
  int CaloRegion;
  float phi;
  float eta;
};

struct hadCalDigiType {
  int id;
  float Et;
  int  CaloRegion;
  float phi;
  float eta;
};

tag CALDIGI {
  int nEmDigi;
  emCalDigiType emDigi[nEmDigi];
  
  int nhadDigi;
  hadCalDigiType hadDigi[nhadDigi]; 
};


tag L2CALEM {
    int    ID;
    int    idObjT;
    float  Et;
    float  phi;
    float  eta;
    float  depth;
    float  z2;
    float  r2;
    float  Et37;
    float  Et57;
    float  Et79;
    float  Etlay[4];
    float  Et33lay[4];
    float  Et77lay[4];
    float  EtHad;
    float  wEta2;
    float  wEta2c;
    float  wEta1;
    float  wEta1mc;
    float  E1st;
    float  E2nd;
    float  Elow;
    float  frac73;
    float  EtLVL1;
    float  eta1st;
    int    qual;
    int    kPart;
    int    T2CR;
    int    T2ZD;
};


struct trtDigiType {
  int Part;   /* part no. */
  int Plane;  /* plane no. */
  int Straw;  /* straw no. */
  int Drift;  /* drift time */
  int TRflag; /* Trans. Radn. flag 1: low thr. 2: hi thr. hit*/
  int kineId;  /* id of kine that caused the track */
};

tag TRTDIGI {
 int         nDigi;
 trtDigiType digi[nDigi];
};

tag TRTTRACK {
 int     trackid;    
 int     fexid;  /* each fex algorythm will be assigned an id */
 float   PtInv;
 float   ZVert;
 float   PhiVert;
 float   CotThet;
 float   errPtInv;
 float   errZVert;      
 float   errPhiVert;    
 float   errCotThet;    
 int     Nhits;         
 int     Nstraw;        
 int     Ntime;         
 int     NTR;           
 int     MaxGap;        /* max. number of adjacent layers w/o hit */   
 int     FirstLayer;    /* innermost layer of track with hit */
};



tag TRTTB {
   int trackId;
   int kineId;
   int iPart;
   int iFlag;
   int nHits;
   float Pt;
   float Phi0;
   float Eta;
   float Z0;
   float d0;
};

struct sctCheatType {
    int id;
    int address;
    float locY;
    float locZ;
    float gloR;
    float gloPhi;
    float gloZ;
    int indKine;
};

tag SCTCHEAT {
  int nCheat;
  sctCheatType cheat[nCheat];
};

struct sctDigiType {
    int id;
    int address; 
    int value;
};

tag SCTDIGI {
  int nDigi;
  sctDigiType digi[nDigi];
};

struct sctGloCoType {
    int id;
    int address;
    float r;
    float dr;
    float phi;
    float dphi;
    float z;
    float dz;
    int quality;
};

tag SCTCOORD {
  int          nGloCo;
  sctGloCoType gloCo[nGloCo];
};

#define MAXTRACKPLANES  20

tag SCTTRACK {
   int trackId;
   int fexId; /* each fex algorythm will be assigned an id */
   float Pt;
   float Phi0;
   float Eta;
   float Z0;
   float chi1sqr;
   float chi2sqr;
   float chi1sqrProb;
   float chi2sqrProb;
   float D0;
   float errPt;
   float errPhi0;
   float errEta;
   float errZ0;
   float errD0;
   int nHitOnTrack;
   int nShared;
   int nPlanes;
   int modList[MAXTRACKPLANES];
   float hitProb[MAXTRACKPLANES];
};

struct sctTbDigiType {
  int id;
  int nKine;
  short kineId[nKine];
  int nHit;
  short hitId[nHit];
};


struct  sctTbClusType {
  int id;
  int nDigi;
  short digId[nDigi];
};

struct sctTbGloCoType {
  int id;
  int nClus;
  short clusId[nClus];
};

struct sctTbTrackType {
  int id;
  int nGloCo;
  short gloCoId[nGloCo];
};

tag SCTTB {

  int nTbDigi;
  sctTbDigiType TbDigi[nTbDigi];

  int nTbClus;
  sctTbClusType TbClus[nTbClus];

  int nTbGloCo;
  sctTbGloCoType TbGloCo[nTbGloCo];
  
  int nTbTrack;
  sctTbTrackType TbTrack[nTbTrack];
};




tag ROI {
  ROIHEAD header;

  CALDIGI calDigi;
  L2CALEM l2CalEm;

  TRTDIGI trtDigi;
  int nTrtTrack;
  TRTTRACK trtTrack[nTrtTrack];  
  TRTTB trtTb[nTrtTrack];  

  int nSctRob;
  SCTDIGI sctRob[nSctRob];
  SCTCHEAT sctCheat; 
  SCTCOORD sctCoord;
  int nSctTrack;
  SCTTRACK sctTrack[nSctTrack];
  SCTTB sctTb;

  KINE kine;
};

tag EVENT {
  EVHEAD header;
  KINE kine;
  int  nroi;
  ROI roi[nroi];
};
