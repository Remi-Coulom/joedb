#ifdef JOEDB_HAS_SSH

#include "joedb/ssh/Session.h"

#include "gtest/gtest.h"

namespace joedb
{
 static constexpr const char *b64_key =
R"RRR(-----BEGIN OPENSSH PRIVATE KEY-----
b3BlbnNzaC1rZXktdjEAAAAACmFlczI1Ni1jdHIAAAAGYmNyeXB0AAAAGAAAABBADubSlG
1EmKeHf40MxSvVAAAAGAAAAAEAAAAzAAAAC3NzaC1lZDI1NTE5AAAAIDEcGEaP6uBECIzA
mko9gVD9DFRjC5kkRpZAvsgGPYFOAAAAkCTRYY8/xyKDf/439HjU7cAGaHT+J8ilIB4fnz
rQY/CG8yT5qRLqeo6yUnlSI+N2arr59OYsY7/8nKTlOnUcrFz/8umS0EKOUg7EQg9yYVaG
QOyBSpFoucn5gIuppkfB4/5FGlM5whgkW+NhtGxzXKR+ZV/HL0OPWEtcibmUu0XDoOky1b
2DG0OJUmvv8AfHvw==
-----END OPENSSH PRIVATE KEY-----)RRR";

 ////////////////////////////////////////////////////////////////////////////
 TEST(ssh, Imported_Key_OK)
 ////////////////////////////////////////////////////////////////////////////
 {
  const ssh::Imported_Key key(b64_key, "joedb");
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(ssh, Imported_Key_bad_passphrase)
 ////////////////////////////////////////////////////////////////////////////
 {
  EXPECT_ANY_THROW
  (
   const ssh::Imported_Key key(b64_key, "y");
  );
 }

 ////////////////////////////////////////////////////////////////////////////
 TEST(ssh, Imported_Key_bad_key)
 ////////////////////////////////////////////////////////////////////////////
 {
  EXPECT_ANY_THROW
  (
   const ssh::Imported_Key key("x", "y");
  );
 }
}

#endif
