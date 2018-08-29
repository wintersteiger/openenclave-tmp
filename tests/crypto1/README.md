Additional Crypto API tests
===========================

**Preparing Test Data**

The certificates were generated using the following openssl commands.

  1. Create Root CA.
      1. openssl genrsa -out RootCA1.key.pem
      2. openssl req -new -x509 -key RootCA1.key.pem -out RootCA1.crt.pem
   
  2. Create Intermediate CA signed by the Root CA.
      1. openssl genrsa -out IntermediateCA1.key.pem
      2. openssl req -new -key IntermediateCA1.key.pem -out IntermediateCA1.csr
      3. openssl x509 -req -in IntermediateCA1.csr -CA RootCA1.crt.pem -CAkey RootCA1.key.pem -CAcreateserial -out IntermediateCA1.crt.pem
   
  3. Create Leaf certificate signed by the Intermediate CA.
      1. openssl genrsa -out Leaf1.key.pem
      2. openssl req -new -key Leaf1.key.pem -out Leaf1.csr
      3. openssl x509 -req -in Leaf1.csr -CA IntermediateCA1.crt.pem -CAkey IntermediateCA1.key.pem -CAcreateserial -out Leaf1.crt.pem

