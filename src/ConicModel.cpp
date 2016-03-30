#include <sstream>
#include <cmath>
#include "ConicModel.hpp"
#include "xml.hpp"

//-----------------------------------------------------------------------------
ConicModel::ConicModel(const ConicModel &mod)
    : IntrinsicModel(mod), m_focal(mod.m_focal), m_distortion(NULL)
{
    if(mod.m_distortion)
        m_distortion = new DistortionPolynom(*mod.m_distortion);
}
//-----------------------------------------------------------------------------
ConicModel::~ConicModel()
{
    if ( m_distortion ) delete m_distortion;
}
//-----------------------------------------------------------------------------
bool ConicModel::GroundToImage(double x, double y, double z, double &c, double &l) const
{
    if(z<=0) return false;
    double t = m_focal / z;
    c = m_cPPA + x*t;
    l = m_lPPA + y*t;

    return (!m_distortion) || m_distortion->ApplyGroundToImage(c,l);
}
//-----------------------------------------------------------------------------
bool ConicModel::ImageToVec(double c, double l, double &x0, double &y0, double &z0, double &x1, double &y1, double &z1) const
{
    if(m_distortion && !m_distortion->ApplyImageToGround(c,l)) return false;
    x0=y0=z0=0.;
    x1 = c - m_cPPA;
    y1 = l - m_lPPA;
    z1 = m_focal;
    return true;
}

//-----------------------------------------------------------------------------
bool ConicModel::Read(TiXmlNode* node)
{
    TiXmlNode* sensor = FindNode(node,"sensor");
    TiXmlNode* image_size =  FindNode(sensor,"image_size");
    m_width = ReadNodeAs<unsigned int>(image_size,"width");
    m_height= ReadNodeAs<unsigned int>(image_size,"height");

    TiXmlNode* ppa = FindNode(sensor,"ppa");
    m_cPPA = ReadNodeAs<double>(ppa,"c");
    m_lPPA = ReadNodeAs<double>(ppa,"l");
    m_focal= ReadNodeAs<double>(ppa,"focale");

    TiXmlNode* disto = FindNode(sensor,"distortion");
    if(m_distortion) delete m_distortion;
    if(disto)
    {
        m_distortion = new DistortionPolynom;
        m_distortion->Read(disto);
    }
    return true;
}
//-----------------------------------------------------------------------------
bool ConicModel::Write(std::ostream& out) const
{
    int prec = out.precision(3);						// Sauvegarde des parametres du flux
    std::ios::fmtflags flags = out.setf(std::ios::fixed);

    out << "  <intrinseque>" << std::endl;

    out << "   <sensor>" << std::endl;
    out << "    <image_size> " << std::endl;
    out << "     <width> " <<  m_width << " </width>" << std::endl;
    out << "     <height> " <<  m_height << " </height>" << std::endl;
    out << "    </image_size>" << std::endl;

    out.precision(3);

    out << "    <ppa>" << std::endl;
    out << "     <c> " <<  m_cPPA << " </c>" << std::endl;
    out << "     <l> " <<  m_lPPA  << " </l>" << std::endl;
    out << "     <focale> " <<  m_focal << " </focale>" << std::endl;
    out << "    </ppa>" << std::endl;

    if(m_distortion) m_distortion->Write(out);

    out << "   </sensor>" << std::endl;

    out << "  </intrinseque>" << std::endl;

    out.precision(prec);		// Restauration des parametres du flux
    out.unsetf(std::ios::fixed);
    out.setf(flags);

    return out.good();
}